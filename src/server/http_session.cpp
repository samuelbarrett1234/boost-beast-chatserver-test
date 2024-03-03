#include "http_session.hpp"
#include "detail/http_session.hpp"


void begin_http_session(
    std::shared_ptr<ServerState> p_server_state,
    boost::asio::ip::tcp::socket socket)
{
    HttpSession::States::Read::enter(std::make_unique<HttpSession>(
        std::move(p_server_state), std::move(socket)));
}
