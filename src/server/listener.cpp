#include "listener.hpp"
#include "errors.hpp"
#include "http_session.hpp"


std::unique_ptr<Listener> Listener::make(
    boost::asio::io_context& ioc,
    std::shared_ptr<ServerState> _p_server_state,
    boost::asio::ip::tcp::endpoint endpoint)
{
    /*
    * See:
    * https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
    */
    class ListenerWithPublicConstructor :
        public Listener
    {
    public:
        inline ListenerWithPublicConstructor(
            boost::asio::io_context& _ioc,
            std::shared_ptr<ServerState> _p_server_state) :
            Listener(_ioc, std::move(_p_server_state))
        { }
    };

    auto p_listener = std::make_unique<ListenerWithPublicConstructor>(
        ioc, std::move(_p_server_state));

    boost::beast::error_code ec;

    /*
    * NOTES ON GENERAL SOCKET USAGE.
    * A TCP connection is uniquely defined by 5 values: the protocol,
    * the source address and port, and the destination address and
    * port.
    * 1. `open` fixes the protocol,
    * 2. `bind` fixes the source address and port,
    * 3. `async_accept`, when accepted, fixes the destination port and
    *    address. At this point, all 5 values are known, so the TCP
    *    connection is uniquely determined, so only at this point do
    *    we actually create the TCP socket.
    *
    * Useful reference:
    * https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ
    */

    p_listener->acceptor.open(endpoint.protocol(), ec);
    SERVER_VALIDATE_ERROR_CODE(ec);

    /*
    * Allow us to keep listening on the same address. This is needed
    * because the TCP acceptor is listening for any input on this
    * port, and the client TCP connections will end up on the same
    * port when their connections are accepted, so we don't want any
    * conflict.
    */
    p_listener->acceptor.set_option(
        boost::asio::socket_base::reuse_address(true), ec);
    SERVER_VALIDATE_ERROR_CODE(ec);

    p_listener->acceptor.bind(std::move(endpoint), ec);
    SERVER_VALIDATE_ERROR_CODE(ec);

    /*
    * Begin process of listening for connections. Constructs
    * a queue for yet-unaccepted connections. Does not call
    * `async_accept`, that happens next.
    */
    p_listener->acceptor.listen(
        boost::asio::socket_base::max_listen_connections, ec);
    SERVER_VALIDATE_ERROR_CODE(ec);

    return p_listener;
}


void Listener::run(std::unique_ptr<Listener> _p_listener)
{
    auto p_listener = _p_listener.get();

    /*
    * By creating a new strand here, this will be used as the
    * executor for the newly-created socket upon completion
    * of the `async_accept`. This will effectively serialise
    * the whole `HttpSession` created in `on_accept`.
    */
    p_listener->acceptor.async_accept(
        boost::asio::make_strand(p_listener->ioc),
        [p_listener = std::move(_p_listener)](
            boost::beast::error_code ec,
            boost::asio::ip::tcp::socket socket) mutable
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

    /*
    * By creating a new strand here, this will be used as the
    * executor for the newly-created socket upon completion
    * of the `async_accept`. This will effectively serialise
    * the whole `HttpSession` created in `on_accept`.
    */
    p_listener->acceptor.async_accept(
        boost::asio::make_strand(p_listener->ioc),
        [p_listener = std::move(_p_listener)](
            boost::beast::error_code ec,
            boost::asio::ip::tcp::socket socket) mutable
        {
            on_accept(std::move(p_listener), std::move(ec), std::move(socket));
        }
    );
}
