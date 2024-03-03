#ifndef SERVER_HTTP_SESSION_HPP
#define SERVER_HTTP_SESSION_HPP


#include <memory>
#include <boost/asio/ip/tcp.hpp>


namespace server
{


struct Server;  // forward declaration


}  // namespace server


namespace http
{


void start_session(
    std::shared_ptr<server::Server> p_server_state,
    boost::asio::ip::tcp::socket socket);


}  // namespace http


#endif  // SERVER_HTTP_SESSION_HPP
