#pragma once

#include <cstdlib>

#include <raylib.h>

inline bool aabb(Vector3 entityPosition, float entityWidth,
                 Vector3 targetPosition) {
  // AABB collision detection on the XZ plane
  return (entityPosition.x - entityWidth <= targetPosition.x &&
          entityPosition.x + entityWidth >= targetPosition.x &&
          entityPosition.z - entityWidth <= targetPosition.z &&
          entityPosition.z + entityWidth >= targetPosition.z);
}

inline float randRange(float min, float max) {
  return min + (static_cast<float>(rand()) /
                (static_cast<float>(RAND_MAX / (max - min))));
}