#ifndef SERVER_LISTENER_HPP
#define SERVER_LISTENER_HPP


#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>


class ServerState;  // forward declaration


struct Listener
{
    boost::asio::io_context& ioc;
    boost::asio::ip::tcp::acceptor acceptor;
    std::shared_ptr<ServerState> p_server_state;

    static std::unique_ptr<Listener> make(
        boost::asio::io_context& ioc,
        std::shared_ptr<ServerState> p_server_state,
        boost::asio::ip::tcp::endpoint endpoint);

    static void run(std::unique_ptr<Listener> p_listener);

private:
    inline Listener(
        boost::asio::io_context& _ioc, std::shared_ptr<ServerState> _p_server_state) :
        ioc(_ioc),
        acceptor(_ioc),
        p_server_state(std::move(_p_server_state))
    { }

    static void on_accept(
        std::unique_ptr<Listener> p_listener,
        boost::beast::error_code ec,
        boost::asio::ip::tcp::socket socket);
};


#endif  // SERVER_LISTENER_HPP
