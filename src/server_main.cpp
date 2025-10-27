#include <iostream>
#include "boost/asio.hpp"

#include "Server.hpp"

int main()
{
    try
    {
        const uint32_t thread_count = std::thread::hardware_concurrency();
        std::cout << "Using " << thread_count << " worker threads." << std::endl;

        boost::asio::io_context io_context;
        std::shared_ptr<Server> server = std::make_shared<Server>(io_context, 12345); // Listen on port 12345
        server->start_broadcast_loop();
        
        std::vector<std::thread> workers;

        for (uint32_t i = 0; i < thread_count; ++i)
        {
            workers.emplace_back([&io_context] 
            {
                try
                {
                    io_context.run();
                }
                catch (std::exception& e)
                {
                    std::cerr << "Worker threat exception: " << e.what() << std::endl;
                }
            });
        }

        for(auto& t : workers)
        {
            if(t.joinable())
            {
                t.join();
            }
        }

    }
    catch (std::exception &e)
    {
        std::cerr << "Exception in main: " << e.what() << std::endl;
    }
    return 0;
}