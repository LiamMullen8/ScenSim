#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <string>
#include <mutex>

#include "simulation.pb.h"

// class Entity;
struct Entity
{
std::string name;
uint32_t id;
};

class World
{
public:

    World() = default;
    ~World() = default;

    void add_entity(simulation::Entity& proto_entity);
    
    template<typename ModifiedEntity>
    void edit_entity(uint32_t id, ModifiedEntity modified_entity);

    void remove_entity(uint32_t id);

    Entity* get_entity(uint32_t id);

    void populate_world_state(simulation::WorldState& proto_state) const;

private:
    std::map<uint32_t, Entity> entities_;

    mutable std::mutex state_mutex_; 

    uint32_t next_entity_id = 1;

};

#endif // WORLD_HPP


