#include "websocket_session.hpp"
#include "../server.hpp"
#include "../errors.hpp"


namespace websocket
{


void Session::States::Accept::operator()(boost::beast::error_code ec)
{
    SERVER_VALIDATE_ERROR_CODE(ec);

    server::async_join(p_session->p_server_state, p_session);

    States::Read::enter(std::move(p_session));
}


void Session::States::Accept::enter(
    std::shared_ptr<Session> p_session,
    boost::beast::http::request<boost::beast::http::string_body> req)
{
    p_session->stream.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(
            boost::beast::role_type::server));

    p_session->stream.set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::response_type& res)
        {
            res.set(boost::beast::http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING));
        }));

    p_session->stream.async_accept(
        std::move(req),
        Accept{ std::move(p_session) });
}


void Session::States::Read::operator()(
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

    if (ec == boost::asio::error::operation_aborted ||
        ec == boost::beast::websocket::error::closed)
        return;

    SERVER_VALIDATE_ERROR_CODE(ec);

    server::async_broadcast(
        p_session->p_server_state,
        boost::beast::buffers_to_string(p_session->buffer.data()));

    p_session->buffer.consume(bytes_read);

    States::Read::enter(std::move(p_session));
}


void Session::States::Read::enter(std::shared_ptr<Session> p_session)
{
    auto& buffer = p_session->buffer;
    p_session->stream.async_read(
        buffer,
        Read{ std::move(p_session) });
}


void Session::States::Write::operator()(
    boost::beast::error_code ec,
    const size_t bytes_written)
{
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
    States::Write::enter(std::move(p_session));
}


void Session::States::Write::enter(
    std::shared_ptr<Session> p_session)
{
    SERVER_ASSERT(p_session->send_queue.size() > 0);

    auto buffer = boost::asio::buffer(*p_session->send_queue.front());
    p_session->stream.async_write(
        buffer,
        Write{ std::move(p_session) });
}


void Session::States::Enqueue::operator()()
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
    States::Write::enter(std::move(p_session));
}


void Session::States::Enqueue::enter(
    std::shared_ptr<Session> p_session,
    std::shared_ptr<std::string> p_msg)
{
    /*
    * Ensure that parallel access to `this` is
    * prevented.
    * The Websocket stream executor guarantees
    * serial access.
    */
    auto& strand = p_session->strand;
    boost::asio::post(
        strand,
        Enqueue{ std::move(p_session), std::move(p_msg) });
}


}  // namespace websocket
