#include "server.hpp"


namespace server
{


void Server::States::Join::operator()()
{
    p_server_state->sessions.emplace_back(std::move(p_session));
}


void Server::States::Join::enter(
    std::shared_ptr<Server> p_server_state,
    std::shared_ptr<WebsocketSession> p_session)
{
    auto& strand = p_server_state->strand;
    boost::asio::dispatch(
        strand,
        Join{ std::move(p_server_state), std::move(p_session) });
}


void Server::States::Broadcast::operator()()
{
    /*
    * Walk through this array, and at each element try locking
    * the weak pointer to the session. If this fails (because the
    * session has been destroyed) then we want to delete the
    * entry. However, locking a weak pointer is relatively costly,
    * so we want to attempt it exactly once for each element.
    * Therefore we're going to mimic `std::remove_if` here. We
    * cannot actually use `std::remove_if`, as the predicate in
    * that case cannot modify its elements.
    */
    auto begin = p_server_state->sessions.begin(),
        end = p_server_state->sessions.end();
    while (begin < end)
    {
        if (auto p_session = begin->lock())
        {
            ++begin;
            WebsocketSession::async_send(std::move(p_session), p_msg);
        }
        else
        {
            --end;
            std::swap(*begin, *end);
        }
    }
    p_server_state->sessions.erase(
        end, p_server_state->sessions.end());
}


void Server::States::Broadcast::enter(
    std::shared_ptr<Server> p_server_state,
    std::shared_ptr<std::string> p_msg)
{
    auto& strand = p_server_state->strand;
    boost::asio::dispatch(
        strand,
        Broadcast{ std::move(p_server_state), std::move(p_msg) });
}


}  // namespace server
