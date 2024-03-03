# Boost Beast Chatserver Test

This project is just for me to experiment with Boost Asio and Boost Beast.

## Rules

Here are some self-imposed 'rules' that I think are worth abiding by when it comes to Boost Asio/Beast.
1. Model different async objects as finite state machines.
Each state is a completion handler for an async operation, and when performed, launches other async calls.
2. State completion handlers should not throw exceptions.
When something bad happens, it's probably useful to log it somewhere, and then the object can destroy itself by simply _not_ transitioning to a new state.
3. There is an interesting distinction between objects which expose some async APIs (in this case, `ServerState` and `WebsocketSession`) and those which don't.
Those which don't can often be represented using `std::unique_ptr`, and basically have no function other than `make_and_run`.
Those which do will probably have to be represented using a `std::shared_ptr`, and will need to serialise their operations via a `strand` (as the async ops could be called from anywhere.)
Moreover, these are the only objects which will need to return themselves from `make_and_run`.
4. How to bring async testing into this?
We'll probably need template parameters for all stream/connection-like objects.
Doing this is probably quite intrusive, but very useful and necessary.
