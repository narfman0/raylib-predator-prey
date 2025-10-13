#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void initializeTransformSystems(flecs::world &ecs) {
  ecs.system<Position, Velocity>("Transform System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([](flecs::entity, Position &position, Velocity &velocity) {
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