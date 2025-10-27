#include "MessageUtilities.hpp"
#include "World.hpp"

namespace MessageUtilities
{
    void ToProto(const world::Entity *world_entity, simulation::Entity *proto_entity)
    {
        // Clear existing data to ensure a fresh copy
        proto_entity->Clear();

        proto_entity->set_name(world_entity->name);
        proto_entity->set_id(world_entity->id);
        proto_entity->set_type(static_cast<simulation::Entity::Type>(world_entity->type));

        simulation::Vector3 *proto_position = proto_entity->mutable_position();
        proto_position->set_x(world_entity->position.x);
        proto_position->set_y(world_entity->position.y);
        proto_position->set_z(world_entity->position.z);

        simulation::Vector3 *proto_velocity = proto_entity->mutable_velocity();
        proto_velocity->set_x(world_entity->velocity.x);
        proto_velocity->set_y(world_entity->velocity.y);
        proto_velocity->set_z(world_entity->velocity.z);

        simulation::Orientation *proto_orient = proto_entity->mutable_orientation();
        proto_orient->set_psi(world_entity->orientation.psi);
        proto_orient->set_theta(world_entity->orientation.theta);
        proto_orient->set_phi(world_entity->orientation.phi);
    }

    void FromProto(const simulation::Entity *proto_entity, world::Entity &world_entity)
    {
        world_entity.name = proto_entity->name();
        world_entity.id = proto_entity->id();
        world_entity.type = static_cast<world::Entity::Type>(proto_entity->type());

        if (proto_entity->has_position())
        {
            world_entity.position.x = proto_entity->position().x();
            world_entity.position.y = proto_entity->position().y();
            world_entity.position.z = proto_entity->position().z();
        }
        if (proto_entity->has_velocity())
        {
            world_entity.velocity.x = proto_entity->velocity().x();
            world_entity.velocity.y = proto_entity->velocity().y();
            world_entity.velocity.z = proto_entity->velocity().z();
        }
        if (proto_entity->has_orientation())
        {
            world_entity.orientation.psi = proto_entity->orientation().psi();
            world_entity.orientation.theta = proto_entity->orientation().theta();
            world_entity.orientation.phi = proto_entity->orientation().phi();
        }
    }
}
