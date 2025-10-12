#pragma once

#include <raymath.h>

#include "globals.h"

struct PredatorTag {
  bool filler;
};

struct PreyTag {
  bool filler;
};

struct TransformComponent {
  Vector3 position;
  Vector3 velocity;
};

struct EnergyComponent {
  float energy = spawnEnergy / 2.0f;
};