#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <thread>

#include "simulation.pb.h"

class Session;

using boost::asio::ip::tcp;

class Server
{
public:
    Server(boost::asio::io_context &io_context, short port);
    ~Server() {}

    void handle_client_update(const simulation::NetPacket &packet);

    void broadcast_packet(const std::string &serialized_packet);

    void remove_session(std::shared_ptr<Session> session_ptr);

private:
    void do_accept();

    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions_;
    std::mutex sessions_mutex_;
};

#endif // SERVER_HPP