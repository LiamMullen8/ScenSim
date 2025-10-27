#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <string>
#include <mutex>
#include <functional>

#include "simulation.pb.h"

namespace world
{
    struct Vector3
    {
        float x;
        float y;
        float z;
    };

    struct Orientation
    {
        float psi;
        float theta;
        float phi;
    };

    // class Entity;
    struct Entity
    {
        std::string name;
        uint32_t id;
        enum Type
        {
            FIGHTER = 0,
            BOMBER = 1,
            SAM,
            DRONE,
            SATELLITE
        };

        Type type;
        Vector3 position;
        Vector3 velocity;
        Orientation orientation;
    };
}

class World
{
public:
    World() = default;
    ~World() = default;

    uint32_t add_entity(world::Entity &new_world_entity);

    void edit_entity(world::Entity &world_entity);

    void remove_entity(uint32_t id);

    world::Entity* get_entity_by_id(const uint32_t id);
    world::Entity* get_entity_by_name(const std::string &name);

    void populate_world_state(simulation::WorldState &proto_state) const;

private:
    std::map<uint32_t, world::Entity> entities_;

    mutable std::mutex state_mutex_;

    uint32_t next_entity_id = 1;
};

#endif // WORLD_HPP
