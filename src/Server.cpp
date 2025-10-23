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
{
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

void Server::unicast_packet(std::shared_ptr<Session> session_ptr, const std::string &serialized_packet)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    if (session_ptr->socket().is_open())
    {
        session_ptr->send_packet(serialized_packet);
    }
}

void Server::handle_client_update(std::shared_ptr<Session> session_ptr, const simulation::Packet &packet)
{
    // Ensure Session exists for client
    auto it = session_entities_map_.find(session_ptr.get());
    assert(it != session_entities_map_.end());

    simulation::Packet response;
    response.set_source(simulation::Packet_Source_SERVER);

    if (packet.has_entity())
    {
        auto proto_entity = packet.entity();
        auto session_entity = std::find(it->second.begin(), it->second.end(), proto_entity.id());

        simulation::Entity *new_proto_entity = response.mutable_entity();

        switch (proto_entity.action())
        {
        case simulation::Entity_Action_CREATE:
        {
            if (session_entity != it->second.end())
            {
                std::cout << "Duplicate entity" << std::endl;
            }
            else
            {
                // Add to World
                uint32_t new_id = world_state_->add_entity(proto_entity);

                // Add to session->entities map
                it->second.push_back(new_id);

                // Send ACK to client
                Entity *world_entity = world_state_->get_entity_by_id(new_id);
                new_proto_entity->set_name(world_entity->name);
                new_proto_entity->set_id(world_entity->id);
                new_proto_entity->set_type(static_cast<simulation::Entity::Type>(world_entity->type));
                
                new_proto_entity->set_action(simulation::Entity_Action_ACK);
            }
            break;
        }
        case simulation::Entity_Action_EDIT:
        {
            if (session_entity != it->second.end())
            {
                world_state_->edit_entity(proto_entity);
            }
            else
            {
                std::cout << "Entity does not exist." << std::endl;
            }
            break;
        }
        default:
            break;
        }

        std::string serialized_response;
        response.SerializeToString(&serialized_response);
        unicast_packet(session_ptr, serialized_response);
    }
}

void Server::remove_session(std::shared_ptr<Session> session_ptr)
{
    // Remove all entities of session
    auto it = session_entities_map_.find(session_ptr.get());
    if (it != session_entities_map_.end())
    {
        for (uint32_t id : it->second)
        {
            world_state_->remove_entity(id);
        }

        session_entities_map_.erase(it);
    }

    // Remove session
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.erase(
        std::remove(sessions_.begin(), sessions_.end(), session_ptr),
        sessions_.end());

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