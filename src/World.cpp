#include "World.hpp"

void World::add_entity(simulation::Entity &proto_entity)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    Entity new_entity;
    new_entity.name = proto_entity.name();
    new_entity.id = proto_entity.id();

    entities_[new_entity.id] = new_entity;
}

template <typename ModifiedEntity>
void World::edit_entity(uint32_t id, ModifiedEntity modified_entity)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    auto it = entities_.find(id);
    if (it != entities_.end())
    {
        modified_entity(it->second);
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

Entity *World::get_entity(uint32_t id)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    auto it = entities_.find(id);
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
