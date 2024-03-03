#include "server.hpp"
#include "detail/server.hpp"


namespace server
{


std::shared_ptr<server::Server> make(
    boost::asio::io_context& ioc)
{
    return std::make_shared<server::Server>(ioc);
}


void async_join(
    std::shared_ptr<server::Server> p_server_state,
    std::shared_ptr<WebsocketSession> p_session)
{
    server::Server::States::Join::enter(
        std::move(p_server_state), std::move(p_session));
}


void async_broadcast(
    std::shared_ptr<server::Server> p_server_state,
    std::string msg)
{
    server::Server::States::Broadcast::enter(
        std::move(p_server_state), std::make_shared<std::string>(std::move(msg)));
}


}  // namespace server
