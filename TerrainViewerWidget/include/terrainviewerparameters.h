#ifndef TERRAINVIEWERPARAMETERS_H
#define TERRAINVIEWERPARAMETERS_H

namespace TerrainViewer
{

/**
 * \brief Palette for the terrain
 */
enum class Palette
{
	texture = 0,
	white = 1,
	demScreen = 2,
	environment = 3
};

/**
 * \brief Shading of the terrain
 */
enum class Shading
{
	normal = 0,
	uniformLightBasic = 1,
	uniformLight = 2,
	directionalLight = 3,
	slope = 4
};

/**
 * \brief A set of parameters for TerrainViewerWidget
 */
struct Parameters
{
	/**
	 * \brief Palette used to color the terrain
	 */
	Palette palette;

	/**
	 * \brief Shading method
	 */
	Shading shading;

	/**
	 * \brief Display the terrain as a wire-frame
	 */
	bool wireFrame;
	
	/**
	 * \brief Level of details
	 */
	float pixelsPerTriangleEdge;

	/**
	 * \brief Time step for the water simulation
	 */
	float timeStep;

	/**
	 * \brief Iterations of the water simulation per frame displayed in the viewer
	 */
	int iterationsPerFrame;

	/**
	 * \brief Whether the water bounces on borders or go through them
	 */
	bool bounceOnBorders;

	/**
	 * \brief Initial water level when starting the simulation
	 */
	float initialWaterLevel;

	/**
	 * \brief Amount of rain per period of time
	 */
	float rainRate;

	/**
	 * \brief Amount of water that evaporate per period of time
	 */
	float evaporationRate;
};

}

#endif // TERRAINVIEWERPARAMETERS_H