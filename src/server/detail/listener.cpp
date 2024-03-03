#include "listener.hpp"
#include "../http_session.hpp"
#include "../errors.hpp"


namespace listener
{


void Listener::States::Accept::operator()(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::socket socket)
{
    SERVER_VALIDATE_ERROR_CODE(ec);

    http::start_session(
        p_listener->p_server_state, std::move(socket));

    Accept::enter(std::move(p_listener));
}


void Listener::States::Accept::enter(
    std::unique_ptr<Listener> p_listener)
{
    /*
    * By creating a new strand here, this will be used as the
    * executor for the newly-created socket upon completion
    * of the `async_accept`. This will effectively serialise
    * the whole `http::Session` created in `on_accept`.
    */
    auto strand = boost::asio::make_strand(p_listener->ioc);

    p_listener->acceptor.async_accept(
        std::move(strand),
        States::Accept{std::move(p_listener)});
}


}  // namespace listener
