#include "server.hpp"
#include "errors.hpp"


std::shared_ptr<ServerState> ServerState::make(boost::asio::io_context& ioc)
{
    return std::make_shared<ServerState>(ioc);
}


void ServerState::async_join(
    std::shared_ptr<ServerState> _p_server_state,
    std::shared_ptr<WebsocketSession> p_session)
{
    auto p_server_state = _p_server_state.get();

    /*
    * Is `post` the best method here, or can we use `defer` etc?
    */
    p_server_state->strand.post(
        [p_server_state = std::move(_p_server_state), p_session = std::move(p_session)]()
        {
            p_server_state->sessions.emplace_back(std::move(p_session));
        });
}


void ServerState::async_broadcast(
    std::shared_ptr<ServerState> _p_server_state,
    std::string msg)
{
    auto p_server_state = _p_server_state.get();

    /*
    * Is `post` the best method here, or can we use `defer` etc?
    */
    p_server_state->strand.post(
        [p_server_state = std::move(_p_server_state),
            p_msg = std::make_shared<std::string>(std::move(msg))]()
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
        });
}
