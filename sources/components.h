#pragma once

#include <box2d/box2d.h>
#include <flecs.h>
#include <raymath.h>

#include "globals.h"

struct b2Body;

struct TargetComponent {
  flecs::entity target;
  float targetDistanceSq = -1.0F;
  Vector3 targetPosition;
  int framesTillNextTargetSearch;
};

struct PredatorTag {
  bool filler;
};

struct PreyTag {
  bool filler;
};


struct Position : Vector3 {};
struct Velocity : Vector3 {};

struct Energy {
  float energy = spawnEnergy / 2.0f;
};
struct PhysicsBody {
  b2Body *body = nullptr;
};