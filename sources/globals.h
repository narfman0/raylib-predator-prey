#pragma once

const int gridSize = 100;
const int initialPrey = gridSize;
const int initialPredators = initialPrey / 4;
const float maxEnergy = 10.0F;
const float initialEnergy = maxEnergy / 2.0f;
const float speed = 2.0F;
const float preyEnergyGainFactor = 0.5F;
const float predatorEnergyLossFactor = 0.7F;
const float predatorEnergyGainAmount = maxEnergy * 0.8F;
const float gridSizeF = (float)gridSize;
const float gridSizeHalfF = (float)gridSize / 2.0F;
const int targetFps = 60;
const int maxEntities = 800;