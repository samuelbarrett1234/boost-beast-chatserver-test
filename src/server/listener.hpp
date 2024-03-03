#ifndef SERVER_LISTENER_HPP
#define SERVER_LISTENER_HPP


#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>


namespace server
{


struct Server;  // forward declaration


}  // namespace server


void make_and_run_listener(
    boost::asio::io_context& ioc,
    std::shared_ptr<server::Server> p_server_state,
    boost::asio::ip::tcp::endpoint endpoint);


#endif  // SERVER_LISTENER_HPP
