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

    // Public interface for sending message to server
    void send_command(const std::string &command);

private:

    // Establish connection to server
    void do_connect(const tcp::resolver::results_type& endpoints);

    // Read message from server
    void do_read_header();
    void do_read_body(std::size_t body_size);

    // Send message to server
    void do_write();

    enum { HEADER_SIZE = 4 };
    char incoming_header_[HEADER_SIZE];
    std::string incoming_body_;

    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    std::deque<std::string> write_msgs_;
    std::string read_buffer_;
};
