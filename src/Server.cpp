#include "Server.hpp"
#include "Session.hpp"
#include "World.hpp"

Server::Server(boost::asio::io_context &io_context, short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), world_state_(new World())
{
    std::cout << "Server listening on port " << port << std::endl;
    do_accept();
}

Server::~Server() 
{}


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


void Server::handle_client_update(std::shared_ptr<Session> session_ptr, const simulation::Packet &packet)
{  
    if(packet.has_entity())
    {
        if(packet.entity().action() == simulation::Entity::CREATE)
        {
            auto it = session_entities_map_.find(session_ptr.get());
            if(it != session_entities_map_.end())
            {
                auto session_entity = std::find(it->second.begin(), it->second.end(), packet.entity().id());
                if(session_entity != it->second.end())
                {
                    std::cout << "Duplicate entity" << std::endl;
                }
                else
                {
                    auto proto_entity = packet.entity();

                    world_state_->add_entity(proto_entity);
                    
                    // add to session->entities map
                    it->second.push_back(proto_entity.id());
                }
            }
        }
    }

    simulation::Packet response;
    response.set_source(simulation::Packet::SERVER);

    auto world_snapshot = response.mutable_world_state();
    world_state_->populate_world_state(*world_snapshot);

    std::string serialized_response;
    response.SerializeToString(&serialized_response);
    broadcast_packet(serialized_response);

}


void Server::remove_session(std::shared_ptr<Session> session_ptr)
{
    // Remove all entities of session
    auto it = session_entities_map_.find(session_ptr.get());
    if(it != session_entities_map_.end())
    {
        for(uint32_t id : it->second)
        {
            world_state_->remove_entity(id);
        }

        session_entities_map_.erase(it);
    }

    // Remove session
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(
        std::remove(sessions_.begin(),  sessions_.end(),  session_ptr), 
        sessions_.end()
    );

    std::cout << "Session successfully removed. Active sessions: " << sessions_.size() << std::endl;
}


void Server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                // Create a session for the new client
                auto new_session = std::make_shared<Session>(std::move(socket), *this);
                session_entities_map_[new_session.get()] = {};
                {
                    std::lock_guard<std::mutex> lock(sessions_mutex_);
                    sessions_.push_back(new_session);
                    std::cout << "Client connected. Active sessions: " << sessions_.size() << std::endl;
                }
                sessions_.back()->start();
            }
            else
            {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }

            do_accept();
        });
}