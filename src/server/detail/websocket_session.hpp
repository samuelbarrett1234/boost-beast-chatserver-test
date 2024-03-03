#ifndef SERVER_DETAIL_WEBSOCKET_SESSION_HPP
#define SERVER_DETAIL_WEBSOCKET_SESSION_HPP


#include <memory>
#include <list>
#include <string>
#include <boost/beast.hpp>
#include <boost/asio.hpp>


namespace server
{


struct Server;  // forward declaration


}  // namespace server


namespace websocket
{


struct Session
{
    inline Session(
        std::shared_ptr<server::Server> _p_server_state,
        boost::asio::ip::tcp::socket socket) :
        p_server_state(std::move(_p_server_state)),
        stream(std::move(socket)),
        strand(stream.get_executor())
    { }

    boost::beast::websocket::stream<boost::beast::tcp_stream> stream;
    boost::beast::websocket::stream<boost::beast::tcp_stream>::executor_type strand;
    boost::beast::flat_buffer buffer;
    std::shared_ptr<server::Server> p_server_state;

    /*
    * The `send_queue` is protected by the strand executor of `stream`.
    */
    std::list<std::shared_ptr<std::string>> send_queue;

    struct States
    {
        struct Accept
        {
            std::shared_ptr<Session> p_session;

            void operator()(boost::beast::error_code ec);

            static void enter(
                std::shared_ptr<Session> p_session,
                boost::beast::http::request<boost::beast::http::string_body> req);
        };

        struct Read
        {
            std::shared_ptr<Session> p_session;

            void operator()(
                boost::beast::error_code ec,
                size_t bytes_read);

            static void enter(std::shared_ptr<Session> p_session);

        };

        struct Write
        {
            std::shared_ptr<Session> p_session;

            void operator()(
                boost::beast::error_code ec,
                size_t bytes_written);

            static void enter(
                std::shared_ptr<Session> p_session);

        };

        struct Enqueue
        {
            std::shared_ptr<Session> p_session;
            std::shared_ptr<std::string> p_msg;

            void operator()();

            static void enter(
                std::shared_ptr<Session> p_session,
                std::shared_ptr<std::string> p_msg);
        };
    };
};


}  // namespace websocket


#endif  // SERVER_DETAIL_WEBSOCKET_SESSION_HPP
