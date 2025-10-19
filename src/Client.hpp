#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <thread>

using boost::asio::ip::tcp;

class Client
{
public:
    Client(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints);
    ~Client();

    void send(const std::string &msg);

private:

    void do_connect(const tcp::resolver::results_type& endpoints);

    void do_read();

    void do_write();

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    std::deque<std::string> write_msgs_;
    std::string read_buffer_;
};
