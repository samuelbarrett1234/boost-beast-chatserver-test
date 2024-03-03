#include "websocket_session.hpp"
#include "detail/websocket_session.hpp"


namespace websocket
{


std::shared_ptr<Session> start(
    std::shared_ptr<server::Server> p_server_state,
    boost::asio::ip::tcp::socket socket,
    boost::beast::http::request<boost::beast::http::string_body> req)
{
    /*
    * Create session.
    */
    auto p_session = std::make_shared<Session>(
        std::move(p_server_state), std::move(socket));

    /*
    * Immediately look to asynchronously finalise the websocket upgrade.
    */
    Session::States::Accept::enter(p_session, std::move(req));

    return p_session;
}


void async_send(
    std::shared_ptr<Session> p_session,
    std::shared_ptr<std::string> p_msg)
{
    /*
    * Asynchronously enqueue the message.
    */
    Session::States::Enqueue::enter(std::move(p_session), std::move(p_msg));
}


}  // namespace websocket
