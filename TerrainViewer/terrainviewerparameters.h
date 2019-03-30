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
	demScreen = 2
};

/**
 * \brief Shading of the terrain
 */
enum class Shading
{
	normal = 0,
	uniformLightBasic = 1,
	uniformLight = 2,
	directionalLight = 3
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
};

}

#endif // TERRAINVIEWERPARAMETERS_H