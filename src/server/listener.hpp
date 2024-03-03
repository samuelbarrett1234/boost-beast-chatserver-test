#ifndef SERVER_LISTENER_HPP
#define SERVER_LISTENER_HPP


#include <memory>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>


class ServerState;  // forward declaration


void make_and_run_listener(
    boost::asio::io_context& ioc,
    std::shared_ptr<ServerState> p_server_state,
    boost::asio::ip::tcp::endpoint endpoint);


#endif  // SERVER_LISTENER_HPP
