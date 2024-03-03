#ifndef SERVER_HTTP_SESSION_HPP
#define SERVER_HTTP_SESSION_HPP


#include <memory>
#include <boost/asio/ip/tcp.hpp>


namespace server
{


struct Server;  // forward declaration


}  // namespace server


void begin_http_session(
    std::shared_ptr<server::Server> p_server_state,
    boost::asio::ip::tcp::socket socket);


#endif  // SERVER_HTTP_SESSION_HPP
