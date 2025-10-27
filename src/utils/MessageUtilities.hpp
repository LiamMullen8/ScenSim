#ifndef MESSAGEUTILITIES_HPP
#define MESSAGEUTILITIES_HPP

#include "simulation.pb.h"
#include "World.hpp"

namespace MessageUtilities
{
    void ToProto(const world::Entity* world_entity, simulation::Entity* proto_entity);
    void FromProto(const simulation::Entity* proto_entity, world::Entity& world_entity);
}

#endif // MESSAGEUTILITIES_HPP
