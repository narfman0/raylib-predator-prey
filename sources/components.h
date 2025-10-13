#pragma once

#include <raymath.h>

#include "globals.h"

struct PredatorTag {
  bool filler;
};

struct PreyTag {
  bool filler;
};

struct Position : Vector3 {};
struct Velocity : Vector3 {};

struct EnergyComponent {
  float energy = spawnEnergy / 2.0f;
};