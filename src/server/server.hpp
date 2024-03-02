#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP


#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>


class WebsocketSession;  // forward declaration


class ServerState
{
public:
    static std::shared_ptr<ServerState> make(boost::asio::io_context& ioc);

    /*
    * Rather than use a mutex, all of these functions are async instead.
    * This means we can use the strand rather than block.
    */
    static void async_join(
        std::shared_ptr<ServerState> p_server_state,
        std::shared_ptr<WebsocketSession> p_session);

    static void async_broadcast(
        std::shared_ptr<ServerState> p_server_state,
        std::string msg);

private:
    inline ServerState(boost::asio::io_context& ioc) :
        strand(ioc)
    { }

    /*
    * This strand governs access to `sessions`.
    */
    boost::asio::strand<boost::asio::io_context> strand;
    std::vector<std::weak_ptr<WebsocketSession>> sessions;
};


#endif  // SERVER_SERVER_HPP
