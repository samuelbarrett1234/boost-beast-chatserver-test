#ifndef SERVER_WEBSOCKET_SESSION_HPP
#define SERVER_WEBSOCKET_SESSION_HPP


#include <memory>
#include <string>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>


namespace server
{


struct Server;  // forward declaration


}  // namespace server


namespace websocket
{


struct Session;


std::shared_ptr<Session> start(
    std::shared_ptr<server::Server> p_server_state,
    boost::asio::ip::tcp::socket socket,
    boost::beast::http::request<boost::beast::http::string_body> req);


void async_send(
    std::shared_ptr<Session> p_session,
    std::shared_ptr<std::string> p_msg);


}  // namespace websocket


#endif  // SERVER_WEBSOCKET_SESSION_HPP
