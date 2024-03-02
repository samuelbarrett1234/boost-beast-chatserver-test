#ifndef SERVER_WEBSOCKET_SESSION_HPP
#define SERVER_WEBSOCKET_SESSION_HPP


#include <memory>
#include <list>
#include <string>
#include <boost/beast.hpp>
#include <boost/asio.hpp>


class ServerState;  // forward declaration


struct WebsocketSession
{
    static inline std::shared_ptr<WebsocketSession> make(
        std::shared_ptr<ServerState> p_server_state,
        boost::asio::ip::tcp::socket socket)
    {
        return std::make_shared<WebsocketSession>(
            std::move(p_server_state), std::move(socket));
    }

    static inline void run(std::shared_ptr<WebsocketSession> p_session)
    {
        do_read(std::move(p_session));
    }

    static void async_send(
        std::shared_ptr<WebsocketSession> p_session,
        std::shared_ptr<std::string> p_msg);

private:
    inline WebsocketSession(
        std::shared_ptr<ServerState> _p_server_state,
        boost::asio::ip::tcp::socket socket) :
        p_server_state(std::move(_p_server_state)),
        stream(std::move(socket))
    { }

    boost::beast::websocket::stream<boost::beast::tcp_stream> stream;
    boost::beast::flat_buffer buffer;
    boost::beast::websocket::request_parser<boost::beast::websocket::string_body> parser;
    std::shared_ptr<ServerState> p_server_state;

    /*
    * The `send_queue` is protected by the strand executor of `stream`.
    */
    std::list<std::shared_ptr<std::string>> send_queue;

    static void on_accept(
        std::shared_ptr<WebsocketSession> p_session,
        boost::beast::error_code ec);

    static void on_read(
        std::shared_ptr<WebsocketSession> p_session,
        boost::beast::error_code ec,
        size_t bytes_read);

    static void on_write(
        std::shared_ptr<WebsocketSession> p_session,
        boost::beast::error_code ec,
        size_t bytes_written);
};


#endif  // SERVER_WEBSOCKET_SESSION_HPP
