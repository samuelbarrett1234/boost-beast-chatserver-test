#include "http_session.hpp"
#include "detail/http_session.hpp"


namespace http
{


void start_session(
    std::shared_ptr<server::Server> p_server_state,
    boost::asio::ip::tcp::socket socket)
{
    Session::States::Read::enter(std::make_unique<Session>(
        std::move(p_server_state), std::move(socket)));
}


}  // namespace http
