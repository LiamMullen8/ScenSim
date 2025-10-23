#include "World.hpp"

uint32_t World::add_entity(simulation::Entity &proto_entity)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    uint32_t new_id = next_entity_id++;

    Entity new_entity;
    new_entity.name = proto_entity.name();
    new_entity.id = new_id;

    entities_[new_id] = new_entity;

    return new_id;
}

void World::edit_entity(simulation::Entity &proto_entity)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    auto it = entities_.find(proto_entity.id());
    if (it != entities_.end())
    {
        // Update entity with fields from proto entity
        it->second.name = proto_entity.name();
        it->second.type = static_cast<Entity::Type>(proto_entity.type());
        // etc...
    }
}

void World::remove_entity(uint32_t id)
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (entities_.erase(id))
    {
        std::cout << "World: Removed entity " << id << "\n";
    }
}

Entity *World::get_entity_by_id(const uint32_t id)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    Entity *entity = nullptr;

    auto it = entities_.find(id);
    if (it != entities_.end())
    {
        entity = &(it->second);
    }

    return entity;
}

Entity *World::get_entity_by_name(const std::string &name)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    auto it = std::find_if(entities_.begin(), entities_.end(),
                           [name](const std::pair<uint32_t, Entity> &pair)
                           { return pair.second.name == name; });
    if (it != entities_.end())
    {
        return &it->second;
    }

    return nullptr;
}

void World::populate_world_state(simulation::WorldState &proto_state) const
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    for (const auto &pair : entities_)
    {
        const Entity &entity = pair.second;

        simulation::Entity *proto_entity = proto_state.add_entities();
        proto_entity->set_id(entity.id);
        proto_entity->set_name(entity.name);
    }
}
