#ifndef FLOW_H
#define FLOW_H

#include <vector>

#include "terrain.h"

namespace TerrainViewer
{

class WaterSimulation
{
public:
	WaterSimulation();

	/**
	 * \brief Return the current water map
	 * \return The current water map
	 */
	const std::vector<float>& waterMap() const;

	void initSimulation(const Terrain& terrain);
	
	void computeIteration();

private:
	float m_initialWaterLevel;
	float m_rainRate;
	float m_evaporationRate;
	float m_timeStep;
	
	Terrain m_terrain;

	// Height of water on each cell of the terrain
	std::vector<float> m_waterMap;

	// Flow in each direction on each cell
	// In this order: left, right, top, bottom
	std::array<std::vector<float>, 4> m_outFlow;
};

}

#endif // FLOW_H