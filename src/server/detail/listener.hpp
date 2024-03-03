#ifndef SERVER_DETAIL_LISTENER_HPP
#define SERVER_DETAIL_LISTENER_HPP


#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>


namespace server
{


struct Server;  // forward declaration


}  // namespace server


namespace listener
{


struct Listener
{
    inline Listener(
        boost::asio::io_context& _ioc, std::shared_ptr<server::Server> _p_server_state) :
        ioc(_ioc),
        acceptor(_ioc),
        p_server_state(std::move(_p_server_state))
    { }

    boost::asio::io_context& ioc;
    boost::asio::ip::tcp::acceptor acceptor;
    std::shared_ptr<server::Server> p_server_state;

    struct States
    {
        struct Accept
        {
            std::unique_ptr<Listener> p_listener;

            void operator()(
                boost::beast::error_code ec,
                boost::asio::ip::tcp::socket socket);

            static void enter(std::unique_ptr<Listener> p_listener);
        };
    };
};


}  // namespace listener


#endif  // SERVER_DETAIL_LISTENER_HPP
