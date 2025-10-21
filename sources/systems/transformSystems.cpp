#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"
#include "physicsSystems.h"

#include <box2d/box2d.h>

void initializeTransformSystems(flecs::world &ecs) {
  // Apply velocities to physics bodies if present, otherwise do manual integration
  ecs.system<Position, Velocity>("Transform System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([](flecs::entity e, Position &position, Velocity &velocity) {
        if (e.has<PhysicsBody>()) {
          auto &pb = e.get_mut<PhysicsBody>();
          b2Vec2 v(velocity.x, velocity.z);
          pb.body->SetLinearVelocity(v);
          return;
        }

        // Fallback: manual integration and boundary clamping for non-physics entities
        position.x += velocity.x * GetFrameTime();
        position.z += velocity.z * GetFrameTime();
        if (position.x > gridSizeHalfF) {
          position.x = gridSizeHalfF;
          velocity.x *= -1;
        }
        if (position.z > gridSizeHalfF) {
          position.z = gridSizeHalfF;
          velocity.z *= -1;
        }
        if (position.x < -gridSizeHalfF) {
          position.x = -gridSizeHalfF;
          velocity.x *= -1;
        }
        if (position.z < -gridSizeHalfF) {
          position.z = -gridSizeHalfF;
          velocity.z *= -1;
        }
      });
}