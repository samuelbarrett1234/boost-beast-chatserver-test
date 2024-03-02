#ifndef SERVER_HTTP_SESSION_HPP
#define SERVER_HTTP_SESSION_HPP


#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>


class ServerState;  // forward declaration


struct HttpSession
{
    static inline std::unique_ptr<HttpSession> make(
        std::shared_ptr<ServerState> p_server_state,
        boost::asio::ip::tcp::socket socket)
    {
        return std::make_unique<HttpSession>(
            std::move(p_server_state), std::move(socket));
    }

    static inline void run(std::unique_ptr<HttpSession> p_session)
    {
        do_read(std::move(p_session));
    }

private:
    inline HttpSession(
        std::shared_ptr<ServerState> _p_server_state,
        boost::asio::ip::tcp::socket socket) :
        p_server_state(std::move(_p_server_state)),
        stream(std::move(socket))
    { }

    boost::beast::tcp_stream stream;
    boost::beast::flat_buffer buffer;
    boost::beast::http::request_parser<boost::beast::http::string_body> parser;
    std::shared_ptr<ServerState> p_server_state;

    static void do_read(
        std::unique_ptr<HttpSession> p_session);

    static void on_read(
        std::unique_ptr<HttpSession> p_session,
        boost::beast::error_code ec,
        size_t bytes_read);

    static void on_write(
        std::unique_ptr<HttpSession> p_session,
        boost::beast::error_code ec,
        size_t bytes_written,
        bool keep_alive);
};


#endif  // SERVER_HTTP_SESSION_HPP
