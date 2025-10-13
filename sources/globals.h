#pragma once

const int gridSize = 500;
const int initialPrey = 1000;
const int initialPredators = 200;
const float spawnEnergy = 10.0F;
const float speed = 2.0F;
const float preyEnergyGainFactor = 0.5F;
const float predatorEnergyLossFactor = 0.7F;
const float predatorEnergyGainAmount = spawnEnergy * 0.8F;
const float gridSizeF = (float)gridSize;
const float gridSizeHalfF = (float)gridSize / 2.0F;
const int targetFps = 60;
const int maxEntities = 2000;
const float pursuitRange = 10.0F;
const float pursuitRangeSq = pursuitRange * pursuitRange;
const float entityWidth = 1.0F;