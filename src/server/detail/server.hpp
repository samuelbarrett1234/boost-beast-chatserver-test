#ifndef SERVER_DETAIL_SERVER_HPP
#define SERVER_DETAIL_SERVER_HPP


#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "../websocket_session.hpp"


namespace server
{


struct Server
{
    inline Server(boost::asio::io_context& ioc) :
        strand(boost::asio::make_strand(ioc))
    { }

    /*
    * This strand governs access to `sessions`.
    * Rather than use a mutex, we'll use a strand, as this is
    * both more idiomatic and probably more efficient too
    * when there is high contention.
    */
    boost::asio::strand<boost::asio::io_context::executor_type> strand;

    std::vector<std::weak_ptr<websocket::Session>> sessions;

    struct States
    {
        struct Join
        {
            std::shared_ptr<Server> p_server_state;
            std::shared_ptr<websocket::Session> p_session;

            void operator()();

            static void enter(
                std::shared_ptr<Server> p_server_state,
                std::shared_ptr<websocket::Session> p_session);
        };

        struct Broadcast
        {
            std::shared_ptr<Server> p_server_state;
            std::shared_ptr<std::string> p_msg;

            void operator()();

            static void enter(
                std::shared_ptr<Server> p_server_state,
                std::shared_ptr<std::string> p_msg);
        };
    };
};


}  // namespace server


#endif  // SERVER_DETAIL_SERVER_HPP
