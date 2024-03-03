#ifndef SERVER_WEBSOCKET_SESSION_HPP
#define SERVER_WEBSOCKET_SESSION_HPP


#include <memory>
#include <list>
#include <string>
#include <boost/beast.hpp>
#include <boost/asio.hpp>


class ServerState;  // forward declaration


class WebsocketSession
{
public:
    static inline std::shared_ptr<WebsocketSession> make(
        std::shared_ptr<ServerState> p_server_state,
        boost::asio::ip::tcp::socket socket)
    {
        /*
        * See:
        * https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
        */
        class WebsocketSessionWithPublicConstructor :
            public WebsocketSession
        {
        public:
            inline WebsocketSessionWithPublicConstructor(
                std::shared_ptr<ServerState> _p_server_state,
                boost::asio::ip::tcp::socket socket) :
                WebsocketSession(std::move(_p_server_state), std::move(socket))
            { }
        };

        return std::make_shared<WebsocketSessionWithPublicConstructor>(
            std::move(p_server_state), std::move(socket));
    }

    static void run(
        std::shared_ptr<WebsocketSession> p_session,
        boost::beast::http::request<boost::beast::http::string_body> req);

    static void async_send(
        std::shared_ptr<WebsocketSession> p_session,
        std::shared_ptr<std::string> p_msg);

protected:
    inline WebsocketSession(
        std::shared_ptr<ServerState> _p_server_state,
        boost::asio::ip::tcp::socket socket) :
        p_server_state(std::move(_p_server_state)),
        stream(std::move(socket))
    { }

    boost::beast::websocket::stream<boost::beast::tcp_stream> stream;
    boost::beast::flat_buffer buffer;
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
