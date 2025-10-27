#include "Server.hpp"
#include "Session.hpp"
#include "World.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>
#include "utils/MessageUtilities.hpp"

Server::Server(boost::asio::io_context &io_context, short port) :
    io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
    timer_(io_context),
    world_state_(new World())
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

void Server::start_broadcast_loop()
{
    auto self = shared_from_this();

    timer_.expires_from_now(std::chrono::milliseconds(33));

    timer_.async_wait(
        [this, self](boost::system::error_code ec)
        {
            if (!ec)
            {
                broadcast_world_state();
                start_broadcast_loop();
            }
            else if (ec == boost::asio::error::operation_aborted)
            {
                std::cerr << "Broadcast timer error: " << ec.message() << std::endl;
            }
        });
}

void Server::broadcast_world_state()
{
    simulation::Packet packet;
    packet.set_source(simulation::Packet_Source_SERVER);

    simulation::WorldState* proto_state = packet.mutable_world_state();
    world_state_->populate_world_state(*proto_state);

    std::string serialized_packet;
    if(packet.SerializeToString(&serialized_packet))
    {
        broadcast_packet(serialized_packet);
    }
    else
    {
        std::cerr << "Failed to serialize world state" << std::endl;
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
                world::Entity entity;
                MessageUtilities::FromProto(new_proto_entity, entity);

                uint32_t new_id = world_state_->add_entity(entity);

                // Add to session->entities map
                it->second.push_back(new_id);

                // Send ACK to client
                world::Entity *world_entity = world_state_->get_entity_by_id(new_id);
                MessageUtilities::ToProto(world_entity, new_proto_entity);

                new_proto_entity->set_action(simulation::Entity_Action_ACK);
            }
            break;
        }
        case simulation::Entity_Action_EDIT:
        {
            if (session_entity != it->second.end())
            {
                world::Entity entity;
                MessageUtilities::FromProto(new_proto_entity, entity);

                world_state_->edit_entity(entity);

                // Send ACK to client
                world::Entity *world_entity = world_state_->get_entity_by_id(entity.id);
                
                simulation::Entity* new_proto_entity;
                MessageUtilities::ToProto(world_entity, new_proto_entity);

                new_proto_entity->set_action(simulation::Entity_Action_ACK);
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