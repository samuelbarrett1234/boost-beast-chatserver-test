#include "server.hpp"
#include "listener.hpp"
#include <boost/asio/signal_set.hpp>
#include <iostream>


int main(const int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cout << "Usage: `server <address> <port>`" << std::endl;
            return 0;
        }

        const auto address = boost::asio::ip::make_address(argv[1]);
        const auto port = static_cast<unsigned short>(std::atoi(argv[2]));

        boost::asio::io_context ioc;

        make_and_run_listener(
            ioc,
            server::make(ioc),
            boost::asio::ip::tcp::endpoint{ address, port });

        /*
        * Capture interrupt and termination signals.
        */
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait(
            [&ioc](const boost::system::error_code& ec, int)
            {
                /*
                * This will cause `ioc.run` to exit immediately (presumably
                * it cannot interrupt the middle of a completion handler
                * though?) and clears the work queue, which in turn will
                * destroy all associated objects referenced by the handlers.
                */
                ioc.stop();
            }
        );

        /*
        * Blocks for the entirety of the program. Only returns when we receive
        * `SIGINT` or `SIGTERM`.
        */
        ioc.run();

        return 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
