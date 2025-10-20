#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <deque>

using boost::asio::ip::tcp;

// Forward declaration
class Server;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, Server &server);

    virtual ~Session() = default;

    void start();

    void send_packet(const std::string &serialized_packet);

    tcp::socket &socket() { return socket_; }

private:

    // Read proto message from client
    void do_read_header();
    void do_read_body(std::size_t body_size);

    // Send proto message to client
    void do_write();

    tcp::socket socket_;
    Server &server_;

    enum { HEADER_SIZE = 4 };
    char incoming_header_[HEADER_SIZE];
    std::string incoming_body_;

    // Message queue for async writes
    std::deque<std::string> write_msgs_;
};

#endif // SESSION_HPP
