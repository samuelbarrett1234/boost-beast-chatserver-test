#include "websocket_session.hpp"
#include "server.hpp"
#include "errors.hpp"


void WebsocketSession::async_send(
    std::shared_ptr<WebsocketSession> _p_session,
    std::shared_ptr<std::string> p_msg)
{
    auto p_session = _p_session.get();

    /*
    * Ensure that parallel access to `this` is
    * prevented.
    * The Websocket stream executor guarantees
    * serial access.
    */
    boost::asio::post(
        p_session->stream.get_executor(),
        [p_session = std::move(_p_session), p_msg = std::move(p_msg)]()
        {
            p_session->send_queue.push_back(std::move(p_msg));

            /*
            * If this holds, we must already be writing somewhere else.
            */
            if (p_session->send_queue.size() > 1)
                return;

            /*
            * Else, we are not currently writing, so send this immediately.
            */
            auto ps = p_session.get();
            ps->stream.async_write(
                boost::asio::buffer(*ps->send_queue.front()),
                [p_session = std::move(p_session)](
                    boost::beast::error_code ec,
                    const size_t bytes_written)
                {
                    WebsocketSession::on_write(std::move(p_session), ec, bytes_written);
                });
        });
}


void WebsocketSession::on_accept(
    std::shared_ptr<WebsocketSession> _p_session,
    boost::beast::error_code ec)
{
    auto p_session = _p_session.get();

    SERVER_VALIDATE_ERROR_CODE(ec);

    ServerState::async_join(p_session->p_server_state, _p_session);

    p_session->stream.async_read(
        p_session->buffer,
        [p_session = std::move(_p_session)](
            boost::beast::error_code ec,
            const size_t bytes_read)
        {
            WebsocketSession::on_read(std::move(p_session), ec, bytes_read);
        });
}


void WebsocketSession::on_read(
    std::shared_ptr<WebsocketSession> _p_session,
    boost::beast::error_code ec,
    const size_t bytes_read)
{
    /*
    * NOTE: this `on_read` handler is not executed in a strand,
    * because it is in an implicit strand. This is unlike
    * `async_send`, which is part of the public API and can be
    * called from anywhere, which is why that _is_ executed in
    * a strand.
    * Our `p_session->buffer` is not used by the write functions,
    * thus can be used safely here outside of a strand.
    */

    auto p_session = _p_session.get();

    if (ec == boost::asio::error::operation_aborted ||
        ec == boost::beast::websocket::error::closed)
        return;

    SERVER_VALIDATE_ERROR_CODE(ec);

    ServerState::async_broadcast(
        p_session->p_server_state,
        boost::beast::buffers_to_string(p_session->buffer.data(), bytes_read));

    p_session->buffer.consume(bytes_read);

    p_session->stream.async_read(
        p_session->buffer,
        [p_session = std::move(_p_session)](
            boost::beast::error_code ec,
            const size_t bytes_read)
        {
            WebsocketSession::on_read(std::move(p_session), ec, bytes_read);
        });
}


void WebsocketSession::on_write(
    std::shared_ptr<WebsocketSession> _p_session,
    boost::beast::error_code ec,
    const size_t bytes_written)
{
    auto p_session = _p_session.get();

    if (ec == boost::asio::error::operation_aborted ||
        ec == boost::beast::websocket::error::closed)
        return;

    SERVER_VALIDATE_ERROR_CODE(ec);

    p_session->send_queue.erase(p_session->send_queue.begin());

    if (p_session->send_queue.empty())
        return;

    /*
    * Else start immediately on processing the next buffer
    * to send.
    */
    p_session->stream.async_write(
        boost::asio::buffer(*p_session->send_queue.front()),
        [p_session = std::move(p_session)](
            boost::beast::error_code ec,
            const size_t bytes_written)
        {
            WebsocketSession::on_write(std::move(p_session), ec, bytes_written);
        });
}
