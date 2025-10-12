#pragma once

const int gridSize = 100;
const int initialPrey = 100;
const int initialPredators = 25;
const float spawnEnergy = 10.0F;
const float speed = 2.0F;
const float preyEnergyGainFactor = 0.5F;
const float predatorEnergyLossFactor = 0.7F;
const float predatorEnergyGainAmount = spawnEnergy * 0.8F;
const float gridSizeF = (float)gridSize;
const float gridSizeHalfF = (float)gridSize / 2.0F;
const int targetFps = 60;
const int maxEntities = 800;
const float pursuitRange = 20.0F;
const float pursuitRangeSq = pursuitRange * pursuitRange;