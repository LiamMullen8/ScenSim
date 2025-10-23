#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <string>
#include <mutex>
#include <functional>

#include "simulation.pb.h"

// class Entity;
struct Entity
{
    std::string name;
    uint32_t id;
    enum Type
    {
        FIGHTER = 0,
        BOMBER  = 1,
        SAM,
        DRONE,
        SATELLITE
    };

    Type type;
};

class World
{
public:
    World() = default;
    ~World() = default;

    uint32_t add_entity(simulation::Entity &proto_entity);

    void edit_entity(simulation::Entity &proto_entity);

    void remove_entity(uint32_t id);

    Entity *get_entity_by_id(const uint32_t id);
    Entity *get_entity_by_name(const std::string &name);

    void populate_world_state(simulation::WorldState &proto_state) const;

private:
    std::map<uint32_t, Entity> entities_;

    mutable std::mutex state_mutex_;

    uint32_t next_entity_id = 1;
};

#endif // WORLD_HPP
