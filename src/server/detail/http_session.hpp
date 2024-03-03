#ifndef SERVER_DETAIL_HTTP_SESSION_HPP
#define SERVER_DETAIL_HTTP_SESSION_HPP


#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>


namespace server
{


struct Server;  // forward declaration


}  // namespace server


namespace http
{


struct Session
{
    inline Session(
        std::shared_ptr<server::Server> p_server_state,
        boost::asio::ip::tcp::socket socket) :
        p_server_state(std::move(p_server_state)),
        stream(std::move(socket))
    { }

    boost::beast::tcp_stream stream;
    boost::beast::flat_buffer buffer;
    std::shared_ptr<server::Server> p_server_state;

    struct States
    {
        struct Read
        {
            using Parser = boost::beast::http::request_parser<
                boost::beast::http::string_body>;

            std::unique_ptr<Session> p_session;
            std::unique_ptr<Parser> p_parser;

            void operator()(
                boost::beast::error_code ec,
                size_t bytes_read);

            static void enter(
                std::unique_ptr<Session> p_session);
        };

        struct Write
        {
            std::unique_ptr<Session> p_session;
            bool keep_alive;

            void operator()(
                boost::beast::error_code ec,
                size_t bytes_read);

            static void enter(
                std::unique_ptr<Session> p_session,
                boost::beast::http::message_generator msg,
                bool keep_alive);
        };
    };
};


}  // namespace http


#endif  // SERVER_DETAIL_HTTP_SESSION_HPP
