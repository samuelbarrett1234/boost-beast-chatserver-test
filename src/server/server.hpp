#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP


#include <memory>
#include <string>
#include <boost/asio/io_context.hpp>


namespace websocket
{


struct Session;  // forward declaration


}  // namespace websocket


namespace server
{


struct Server;  // forward declaration


std::shared_ptr<Server> start(
    boost::asio::io_context& ioc);


void async_join(
    std::shared_ptr<Server> p_server_state,
    std::shared_ptr<websocket::Session> p_session);


void async_broadcast(
    std::shared_ptr<Server> p_server_state,
    std::string msg);


}  // namespace server


#endif  // SERVER_SERVER_HPP
