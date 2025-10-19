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
    
    virtual ~Session();

    void start();

    void send_message(const std::string &message);

    tcp::socket& socket() {return socket_;}

private:
    void do_read();
    void do_write();

    tcp::socket socket_;
    Server &server_;
    enum { max_length = 1024 };
    char data_[max_length];

    // Message queue for async writes
    std::deque<std::string> write_msgs_;
};

#endif // SESSION_HPP
