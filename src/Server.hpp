#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <thread>

#include "simulation.pb.h"

class Session;
class World;

using boost::asio::ip::tcp;

class Server
{
public:

    Server(boost::asio::io_context &io_context, short port);
    ~Server();

    // Send message to all clients
    void broadcast_packet(const std::string &serialized_packet);

    // Process message from client
    void handle_client_update(std::shared_ptr<Session> session_ptr, const simulation::Packet &packet);
 
    // Remove session on client disconnect
    void remove_session(std::shared_ptr<Session> session_ptr);

private:

    // Accept connection from new client
    void do_accept();

    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions_;
    std::mutex sessions_mutex_;

    std::unique_ptr<World> world_state_;

    std::map<Session*, std::vector<uint32_t>> session_entities_map_;


};

#endif // SERVER_HPP