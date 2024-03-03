#include "http_session.hpp"
#include "websocket_session.hpp"
#include "errors.hpp"
#include <chrono>


void HttpSession::do_read(
    std::unique_ptr<HttpSession> _p_session)
{
    auto p_session = _p_session.get();

    p_session->parser.emplace();
    p_session->parser->body_limit(10000);
    p_session->stream.expires_after(std::chrono::seconds(30));

    boost::beast::http::async_read(
        p_session->stream,
        p_session->buffer,
        *p_session->parser,
        [p_session = std::move(_p_session)](
            boost::beast::error_code ec,
            const size_t bytes_written) mutable
        {
            on_read(std::move(p_session), ec, bytes_written);
        });
}


void HttpSession::on_read(
    std::unique_ptr<HttpSession> _p_session,
    boost::beast::error_code ec,
    const size_t bytes_read)
{
    auto p_session = _p_session.get();

    if (ec = boost::beast::http::error::end_of_stream)
    {
        /*
        * Gracefully close socket.
        */
        p_session->stream.socket().shutdown(
            boost::asio::ip::tcp::socket::shutdown_send, ec);
        return;
    }

    SERVER_VALIDATE_ERROR_CODE(ec);

    /*
    * See if this is a websocket upgrade request.
    */
    if (boost::beast::websocket::is_upgrade(p_session->parser->get()))
    {
        /*
        * Create a websocket session and transfer ownership.
        */
        WebsocketSession::run(WebsocketSession::make(
            std::move(p_session->p_server_state), p_session->stream.release_socket()),
                p_session->parser->release());

        return;
    }

    const auto req = p_session->parser->release();

    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::not_found, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource was not found.";
    res.prepare_payload();

    const bool keep_alive = req.keep_alive();

    boost::beast::async_write(
        p_session->stream,
        boost::beast::http::message_generator(std::move(res)),
        [p_session = std::move(_p_session), keep_alive](
            boost::beast::error_code ec, const size_t bytes_written) mutable
        {
            on_write(
                std::move(p_session),
                ec,
                bytes_written,
                keep_alive);
        });
}


void HttpSession::on_write(
    std::unique_ptr<HttpSession> _p_session,
    boost::beast::error_code ec,
    const size_t bytes_written,
    const bool keep_alive)
{
    auto p_session = _p_session.get();

    if (ec == boost::asio::error::operation_aborted)
        return;

    SERVER_VALIDATE_ERROR_CODE(ec);

    if (!keep_alive)
    {
        /*
        * Gracefully close socket.
        */
        p_session->stream.socket().shutdown(
            boost::asio::ip::tcp::socket::shutdown_send, ec);
        return;
    }

    do_read(std::move(_p_session));
}
