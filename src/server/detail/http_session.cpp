#include "http_session.hpp"
#include "../errors.hpp"
#include "../websocket_session.hpp"


namespace http
{


void Session::States::Read::operator()(
    boost::beast::error_code ec,
    const size_t bytes_read)
{
    if (ec == boost::beast::http::error::end_of_stream)
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
    if (boost::beast::websocket::is_upgrade(p_parser->get()))
    {
        /*
        * Create a websocket session and transfer ownership.
        */
        websocket::start(
            std::move(p_session->p_server_state),
            p_session->stream.release_socket(),
            p_parser->release());

        return;
    }

    const auto req = p_parser->release();

    auto msg = [&]() -> boost::beast::http::message_generator
    {
        /*
        * Construct HTTP response from request.
        * Note that `boost::beast::http::message_generator`
        * is used for type erasure of the response, and also
        * I think automatically exposes the right kind of
        * "buffer view" to be used in `async_write`.
        */

        boost::beast::http::response<boost::beast::http::string_body> res{
            boost::beast::http::status::not_found, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource was not found.";
        res.prepare_payload();
        return res;

    }();

    /*
    * Respond to the request.
    */
    States::Write::enter(std::move(p_session), std::move(msg), req.keep_alive());
}


void Session::States::Read::enter(
    std::unique_ptr<Session> p_session)
{
    auto p_parser = std::make_unique<Parser>();

    p_parser->body_limit(10000);
    p_session->stream.expires_after(std::chrono::seconds(30));

    /*
    * TODO: can we make this more elegant?
    */
    auto& stream = p_session->stream;
    auto& buffer = p_session->buffer;
    auto& parser = *p_parser;
    boost::beast::http::async_read(
        stream,
        buffer,
        parser,
        Read{ std::move(p_session), std::move(p_parser) });
}


void Session::States::Write::operator()(
    boost::beast::error_code ec,
    const size_t bytes_read)
{
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

    /*
    * Else return to `Read` state.
    */
    States::Read::enter(std::move(p_session));
}


void Session::States::Write::enter(
    std::unique_ptr<Session> p_session,
    boost::beast::http::message_generator msg,
    const bool keep_alive)
{
    auto& stream = p_session->stream;
    boost::beast::async_write(
        stream,
        std::move(msg),
        Write{ std::move(p_session), keep_alive });
}


}  // namespace http
