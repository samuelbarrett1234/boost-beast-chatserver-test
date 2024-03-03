#include "listener.hpp"
#include "detail/listener.hpp"
#include "errors.hpp"


void make_and_run_listener(
    boost::asio::io_context& ioc,
    std::shared_ptr<server::Server> p_server_state,
    boost::asio::ip::tcp::endpoint endpoint)
{
    auto p_listener = std::make_unique<Listener>(
        ioc, std::move(p_server_state));

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

    /*
    * Move the `Listener` into the `Accept` state.
    */
    Listener::States::Accept::enter(std::move(p_listener));
}
