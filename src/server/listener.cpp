#include "listener.hpp"
#include "errors.hpp"
#include "http_session.hpp"


std::shared_ptr<Listener> Listener::make(
    boost::asio::io_context& ioc,
    std::shared_ptr<ServerState> _p_server_state,
    boost::asio::ip::tcp::endpoint endpoint)
{
    auto p_listener = std::make_shared<Listener>(ioc, std::move(_p_server_state));

    boost::beast::error_code ec;

    p_listener->acceptor.open(endpiont, ec);
    SERVER_VALIDATE_ERROR_CODE(ec);

    // ?
    p_listener->acceptor.set_option(
        boost::asio::socket_base::reuse_address(true), ec);
    SERVER_VALIDATE_ERROR_CODE(ec);

    // ?
    p_listener->acceptor.bind(endpoint, ec);
    SERVER_VALIDATE_ERROR_CODE(ec);

    // ?
    p_listener->acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    SERVER_VALIDATE_ERROR_CODE(ec);
}


void Listener::run(std::unique_ptr<Listener> _p_listener)
{
    auto p_listener = _p_listener.get();

    // why in its own strand?
    p_listener->acceptor.async_accept(
        boost::asio::make_strand(p_listener->ioc),
        [p_listener = std::move(_p_listener)](
            boost::beast::error_code ec,
            boost::asio::ip::tcp::socket socket)
        {
            on_accept(std::move(p_listener), std::move(ec), std::move(socket));
        }
    );
}


void Listener::on_accept(
    std::unique_ptr<Listener> _p_listener,
    boost::beast::error_code ec,
    boost::asio::ip::tcp::socket socket)
{
    auto p_listener = _p_listener.get();

    SERVER_VALIDATE_ERROR_CODE(ec);

    /*
    * Start a new HTTP session.
    */
    HttpSession::run(HttpSession::make(p_listener->p_server_state, std::move(socket)));

    // why in its own strand?
    p_listener->acceptor.async_accept(
        boost::asio::make_strand(p_listener->ioc),
        [p_listener = std::move(_p_listener)](
            boost::beast::error_code ec,
            boost::asio::ip::tcp::socket socket)
        {
            on_accept(std::move(p_listener), std::move(ec), std::move(socket));
        }
    );
}
