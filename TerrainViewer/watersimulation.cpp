#include "watersimulation.h"

#include "utils.h"

using namespace TerrainViewer;

void incrementWater(float waterIncrement, float timeStep, std::vector<float>& waterMap)
{
	// Simulate rain, increment water level in each cell
	// TODO: use SIMD
#pragma omp parallel for
	for (int i = 0; i < waterMap.size(); i++)
	{
		waterMap[i] += waterIncrement * timeStep;
	}
}

void evaporateWater(float evaporationRate, float timeStep, std::vector<float>& waterMap)
{
	// Simulate evaporation, increment water level in each cell
	// TODO: use SIMD
#pragma omp parallel for
	for (int i = 0; i < waterMap.size(); i++)
	{
		waterMap[i] = std::max(0.0f, waterMap[i] - evaporationRate * timeStep);
	}
}

void computeOutputFlow(const Terrain& terrain,
	                   float timeStep,
	                   std::vector<float>& waterMap,
	                   std::array<std::vector<float>, 4>& outFlow)
{
	const int width = terrain.resolutionWidth();
	const int height = terrain.resolutionHeight();
	const float cellWidth = terrain.cellWidth();
	const float cellHeight = terrain.cellHeight();

#pragma omp parallel for
	for (int j = 0; j < width; j++)
	{
		for (int i = 0; i < height; i++)
		{
			const int left = clamp(j - 1, 0, width - 1);
			const int right = clamp(j + 1, 0, width - 1);
			const int top = clamp(i - 1, 0, height - 1);
			const int bottom = clamp(i + 1, 0, height - 1);

			const int currentCell = terrain.cellIndex(i, j);
			const int leftCell = terrain.cellIndex(i, left);
			const int rightCell = terrain.cellIndex(i, right);
			const int topCell = terrain.cellIndex(top, j);
			const int bottomCell = terrain.cellIndex(bottom, j);

			// Water height in the 4 neighbors
			const auto waterHeight = waterMap[currentCell];
			const auto waterHeightLeft = waterMap[leftCell];
			const auto waterHeightRight = waterMap[rightCell];
			const auto waterHeightTop = waterMap[topCell];
			const auto waterHeightBottom = waterMap[bottomCell];

			// Altitude of the 4 neighbors
			const auto altitude = terrain(i, j);
			const auto altitudeLeft = terrain(i, left);
			const auto altitudeRight = terrain(i, right);
			const auto altitudeTop = terrain(top, j);
			const auto altitudeBottom = terrain(bottom, j);

			// Difference in height
			const auto diffLeft = (altitude + waterHeight) - (altitudeLeft + waterHeightLeft);
			const auto diffRight = (altitude + waterHeight) - (altitudeRight + waterHeightRight);
			const auto diffTop = (altitude + waterHeight) - (altitudeTop + waterHeightTop);
			const auto diffBottom = (altitude + waterHeight) - (altitudeBottom + waterHeightBottom);

			// Compute the change in flow
			const auto flowLeft = std::max(0.0f, outFlow[0][currentCell] + timeStep * diffLeft / cellWidth);
			const auto flowRight = std::max(0.0f, outFlow[1][currentCell] + timeStep * diffRight / cellWidth);
			const auto flowTop = std::max(0.0f, outFlow[2][currentCell] + timeStep * diffTop / cellHeight);
			const auto flowBottom = std::max(0.0f, outFlow[3][currentCell] + timeStep * diffBottom / cellHeight);

			const auto flowSum = flowLeft + flowRight + flowTop + flowBottom;

			if (flowSum > 0.0f)
			{
				// If the sum of the flow is greater than the quantity of water in the cell, flow is scaled down
				const auto K = clamp((waterHeight * cellWidth * cellHeight) / (flowSum * timeStep), 0.0f, 1.0f);

				// Update the flow
				outFlow[0][currentCell] = flowLeft * K;
				outFlow[1][currentCell] = flowRight * K;
				outFlow[2][currentCell] = flowTop * K;
				outFlow[3][currentCell] = flowBottom * K;
			}
			else
			{
				// Update the flow
				outFlow[0][currentCell] = 0.0f;
				outFlow[1][currentCell] = 0.0f;
				outFlow[2][currentCell] = 0.0f;
				outFlow[3][currentCell] = 0.0f;
			}
		}
	}
}

void updateWaterMap(const Terrain& terrain,
					float timeStep,
					std::vector<float>& waterMap,
					std::array<std::vector<float>, 4>& outFlow)
{
	const int width = terrain.resolutionWidth();
	const int height = terrain.resolutionHeight();
	const float cellArea = terrain.cellWidth() * terrain.cellHeight();

#pragma omp parallel for
	for (int j = 0; j < width; j++)
	{
		for (int i = 0; i < height; i++)
		{
			const int left = clamp(j - 1, 0, width - 1);
			const int right = clamp(j + 1, 0, width - 1);
			const int top = clamp(i - 1, 0, height - 1);
			const int bottom = clamp(i + 1, 0, height - 1);

			const int currentCell = terrain.cellIndex(i, j);
			const int leftCell = terrain.cellIndex(i, left);
			const int rightCell = terrain.cellIndex(i, right);
			const int topCell = terrain.cellIndex(top, j);
			const int bottomCell = terrain.cellIndex(bottom, j);

			const auto outFlowTotal = outFlow[0][currentCell]
				                    + outFlow[1][currentCell]
				                    + outFlow[2][currentCell]
				                    + outFlow[3][currentCell];

			const auto inFlowTotal = outFlow[0][rightCell]  // Flow on the left from the right cell
				                   + outFlow[1][leftCell]   // Flow on the right from the left cell
				                   + outFlow[2][bottomCell] // Flow on the top from the bottom cell
				                   + outFlow[3][topCell];   // Flow on the bottom from the top cell

			const auto newHeight = waterMap[currentCell] + ((inFlowTotal - outFlowTotal) * timeStep) / cellArea;

			waterMap[currentCell] = std::max(0.0f, newHeight);
		}
	}
}

void removeWaterOnBorders(const Terrain& terrain, std::vector<float>& waterMap, std::array<std::vector<float>, 4>& outFlow)
{
	const int width = terrain.resolutionWidth();
	const int height = terrain.resolutionHeight();
	
	// Remove water and flow on the borders of the terrain
	for (int i = 0; i < height; i++)
	{
		waterMap[terrain.cellIndex(i, 0)] = 0.0f;
		waterMap[terrain.cellIndex(i, width - 1)] = 0.0f;

		for (auto& outFlowDirection : outFlow)
		{
			outFlowDirection[terrain.cellIndex(i, 0)] = 0.0f;
			outFlowDirection[terrain.cellIndex(i, width - 1)] = 0.0f;
		}
	}
	for (int j = 0; j < width; j++)
	{
		waterMap[terrain.cellIndex(0, j)] = 0.0f;
		waterMap[terrain.cellIndex(height - 1, j)] = 0.0f;

		for (auto& outFlowDirection : outFlow)
		{
			outFlowDirection[terrain.cellIndex(0, j)] = 0.0f;
			outFlowDirection[terrain.cellIndex(height - 1, j)] = 0.0f;
		}
	}
}

WaterSimulation::WaterSimulation() :
	m_initialWaterLevel(0.1f),
	m_rainRate(0.0f),
	m_evaporationRate(1e-4),
	m_timeStep(0.001f),
	m_terrain(0.0, 0.0, 0.0)
{
	
}

const std::vector<float>& WaterSimulation::waterMap() const
{
	return m_waterMap;
}

void WaterSimulation::initSimulation(const Terrain& terrain)
{
	m_terrain = terrain;
	
	const int width = m_terrain.resolutionWidth();
	const int height = m_terrain.resolutionHeight();
	const int totalResolution = width * height;

	m_waterMap.clear();
	m_waterMap.resize(totalResolution, m_initialWaterLevel);

	// Initialize out flow
	for (unsigned int i = 0; i < m_outFlow.size(); i++)
	{
		m_outFlow[i].clear();
		m_outFlow[i].resize(totalResolution, 0.0);
	}
}

void WaterSimulation::computeIteration()
{
	if (!m_waterMap.empty())
	{
		// Increment and evaporate water in each cell
		incrementWater(m_rainRate, m_timeStep, m_waterMap);
		evaporateWater(m_evaporationRate, m_timeStep, m_waterMap);

		// Compute output flow in each cell
		computeOutputFlow(m_terrain, m_timeStep, m_waterMap, m_outFlow);

		// Update water map
		updateWaterMap(m_terrain, m_timeStep, m_waterMap, m_outFlow);

		// Remove water on borders
		removeWaterOnBorders(m_terrain, m_waterMap, m_outFlow);
	}
}
