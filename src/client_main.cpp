#include <boost/asio.hpp>
#include <iostream>
#include "Client.hpp"

using boost::asio::ip::tcp;

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: ScenClient <host> <port>" << std::endl;
            return 1;
        }

        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);

        Client c(io_context, endpoints);

        std::thread t([&io_context](){ io_context.run(); });
        
        std::string line;
        while(std::getline(std::cin, line))
        {
            if(line == "quit") break;

            // send input to server
            c.send_command(line);
        }

        io_context.stop();
        if(t.joinable())
        {
            t.join();
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception in client: " << e.what() << std::endl;
    }

    return 0;
}