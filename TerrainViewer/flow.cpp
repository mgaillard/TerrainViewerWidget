#include "flow.h"

#include "utils.h"

using namespace TerrainViewer;

std::vector<float> TerrainViewer::computeFlowMap(const Terrain& terrain)
{
	const float initialWaterLevel = 0.01f;
	const float waterIncrement = 0.001f;
	const int numberIterations = 128;
	const float timeStep = 0.2f;

	const int width = terrain.resolutionWidth();
	const int height = terrain.resolutionHeight();
	const int totalResolution = width * height;
	const float cellWidth = terrain.cellWidth();
	const float cellHeight = terrain.cellHeight();

	// Quantity of water on each cell
	std::vector<float> waterMap(totalResolution, initialWaterLevel);
	// Flow in each direction on each cell
	// Order: left, right, top, bottom
	std::array<std::vector<float>, 4> outFlow;

	// Average flow going through each cell
	std::vector<float> averageFlow(totalResolution, 0.0);

	// Initialize out flow
	for (unsigned int i = 0; i < outFlow.size(); i++)
	{
		outFlow[i].resize(totalResolution, 0.0);
	}

	// Compute multiple iterations
	for (int iteration = 0; iteration < numberIterations; iteration++)
	{
		// Simulate rain, increment water level in each cell
		for (int i = 0; i < totalResolution; i++)
		{
			waterMap[i] += waterIncrement * timeStep;
		}

		// Compute flow 
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
				const auto flowLeft = std::max(0.0f, diffLeft + timeStep * outFlow[0][currentCell] / cellWidth);
				const auto flowRight = std::max(0.0f, diffRight + timeStep * outFlow[1][currentCell] / cellWidth);
				const auto flowTop = std::max(0.0f, diffTop + timeStep * outFlow[2][currentCell] / cellHeight);
				const auto flowBottom = std::max(0.0f, diffBottom + timeStep * outFlow[3][currentCell] / cellHeight);

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

		// Update water map
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

				const auto newHeight = waterMap[currentCell] + (inFlowTotal - outFlowTotal) * timeStep;

				waterMap[currentCell] = std::max(0.0f, newHeight);

				averageFlow[currentCell] += (inFlowTotal - outFlowTotal) / numberIterations;
			}
		}

		// Remove water on the borders of the terrain
		for (int i = 0; i < height; i++)
		{
			waterMap[terrain.cellIndex(i, 0)] = 0.0f;
			outFlow[0][terrain.cellIndex(i, 0)] = 0.0f;
			outFlow[1][terrain.cellIndex(i, 0)] = 0.0f;
			outFlow[2][terrain.cellIndex(i, 0)] = 0.0f;
			outFlow[3][terrain.cellIndex(i, 0)] = 0.0f;

			waterMap[terrain.cellIndex(i, width - 1)] = 0.0f;
			outFlow[0][terrain.cellIndex(i, width - 1)] = 0.0f;
			outFlow[1][terrain.cellIndex(i, width - 1)] = 0.0f;
			outFlow[2][terrain.cellIndex(i, width - 1)] = 0.0f;
			outFlow[3][terrain.cellIndex(i, width - 1)] = 0.0f;
		}
		for (int j = 0; j < width; j++)
		{
			waterMap[terrain.cellIndex(0, j)] = 0.0f;
			outFlow[0][terrain.cellIndex(0, j)] = 0.0f;
			outFlow[1][terrain.cellIndex(0, j)] = 0.0f;
			outFlow[2][terrain.cellIndex(0, j)] = 0.0f;
			outFlow[3][terrain.cellIndex(0, j)] = 0.0f;

			waterMap[terrain.cellIndex(height - 1, j)] = 0.0f;
			outFlow[0][terrain.cellIndex(height - 1, j)] = 0.0f;
			outFlow[1][terrain.cellIndex(height - 1, j)] = 0.0f;
			outFlow[2][terrain.cellIndex(height - 1, j)] = 0.0f;
			outFlow[3][terrain.cellIndex(height - 1, j)] = 0.0f;
		}
	}

	// Remap between 0 and 1
	const auto itMinMax = std::minmax_element(averageFlow.begin(), averageFlow.end());
	const float minimum = *itMinMax.first;
	const float maximum = *itMinMax.second;
	for (unsigned int i = 0; i < averageFlow.size(); i++)
	{
		averageFlow[i] = (averageFlow[i] - minimum) / (maximum - minimum);
	}

	return averageFlow;
}