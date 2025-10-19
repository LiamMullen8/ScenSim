#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <thread>

#include "simulation.pb.h"

using boost::asio::ip::tcp;

class Client
{
public:
    Client(boost::asio::io_context &io_context, const tcp::resolver::results_type &endpoints);
    ~Client();

    void send_command(const std::string &command);

private:

    void do_connect(const tcp::resolver::results_type& endpoints);

    void do_read_header();
    void do_read_body(std::size_t body_size);

    void do_write();

    enum { HEADER_SIZE = 4 };
    char incoming_header_[HEADER_SIZE];
    std::string incoming_body_;

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    std::deque<std::string> write_msgs_;
    std::string read_buffer_;
};
