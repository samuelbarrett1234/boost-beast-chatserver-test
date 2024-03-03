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
    static inline std::shared_ptr<ServerState> make(boost::asio::io_context& ioc)
    {
        /*
        * See:
        * https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
        */
        class ServerStateWithPublicConstructor :
            public ServerState
        {
        public:
            inline ServerStateWithPublicConstructor(boost::asio::io_context& ioc) :
                ServerState(ioc)
            { }
        };

        return std::make_shared<ServerStateWithPublicConstructor>(ioc);
    }

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

protected:
    inline ServerState(boost::asio::io_context& ioc) :
        strand(boost::asio::make_strand(ioc))
    { }

    /*
    * This strand governs access to `sessions`.
    */
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
    std::vector<std::weak_ptr<WebsocketSession>> sessions;
};


#endif  // SERVER_SERVER_HPP
