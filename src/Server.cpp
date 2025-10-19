#include "Server.hpp"
#include "Session.hpp"

Server::Server(boost::asio::io_context &io_context, short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server listening on port " << port << std::endl;
    do_accept();
}

void Server::broadcast_packet(const std::string &serialized_packet)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    for (const auto &session : sessions_)
    {
        if (session->socket().is_open())
        {
            session->send_packet(serialized_packet);
        }
    }
}

void Server::handle_client_update(const simulation::NetPacket &packet)
{
    std::cout << "Message received from client: " << std::endl;

    if (packet.type() == simulation::NetPacket::CLIENT_INPUT)
    {
        simulation::NetPacket response;
        response.set_type(simulation::NetPacket::SERVER_STATE);

        std::string serialized_response;
        response.SerializeToString(&serialized_response);
        broadcast_packet(serialized_response);
    }
}

void Server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                // Create a new session for the client
                auto new_session = std::make_shared<Session>(std::move(socket), *this);
                {
                    std::lock_guard<std::mutex> lock(sessions_mutex_);
                    sessions_.push_back(new_session);
                    std::cout << "Session successfully created. Active sessions: " << sessions_.size() << std::endl;
                }
                sessions_.back()->start();
            }
            else
            {
                std::cerr << "Acceptor error: " << ec.message() << std::endl;
            }

            // Continue listening for more connections
            do_accept();
        });
}

void Server::remove_session(std::shared_ptr<Session> session_ptr)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(std::remove_if(sessions_.begin(), sessions_.end(),
                                   [&session_ptr](const std::shared_ptr<Session> &s)
                                   {
                                       return s.get() == session_ptr.get();
                                   }),
                    sessions_.end());
    std::cout << "Session successfully removed. Active sessions: " << sessions_.size() << std::endl;
}
