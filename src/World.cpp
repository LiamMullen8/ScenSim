#include "World.hpp"
#include "utils/MessageUtilities.hpp"

uint32_t World::add_entity(world::Entity &new_world_entity)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    uint32_t new_id = next_entity_id++;

    entities_[new_id] = new_world_entity;

    return new_id;
}

void World::edit_entity(world::Entity &world_entity)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    auto it = entities_.find(world_entity.id);
    if (it != entities_.end())
    {
        it->second = world_entity;
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

world::Entity* World::get_entity_by_id(const uint32_t id)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    world::Entity *entity = nullptr;

    auto it = entities_.find(id);
    if (it != entities_.end())
    {
        entity = &(it->second);
    }

    return entity;
}

world::Entity* World::get_entity_by_name(const std::string &name)
{
    std::lock_guard<std::mutex> lock(state_mutex_);

    auto it = std::find_if(entities_.begin(), entities_.end(),
                           [name](const std::pair<uint32_t, world::Entity> &pair)
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
        simulation::Entity *proto_entity = proto_state.add_entities();        
        MessageUtilities::ToProto(&pair.second, proto_entity);
    }
}
