#ifndef OCCLUSION_H
#define OCCLUSION_H

#include <vector>
#include <bitset>

#include "terrain.h"
#include "terrainviewerparameters.h"

namespace TerrainViewer
{

struct HorizonAngles
{
	// A type to specify which direction is enabled when computing ambient occlusion
	using EnabledDirections = std::bitset<16>;

	// Horizon angles directions
	static const std::array<std::pair<int, int>, 16> directions;

	// Array of 16 horizon angles value initialized
	std::array<float, 16> angles{};
};

/**
 * \brief Compute the horizon angles on a terrain with a fast algorithm.
 *		  See horizonAngleScan
 * \param terrain A terrain
 * \return The horizon angles in each cell of the terrain
 */
std::vector<HorizonAngles> computeHorizonAngles(const Terrain& terrain);

std::vector<float> ambientOcclusionBasic(const Terrain& terrain, const std::vector<HorizonAngles>& horizonAngles);

std::vector<float> ambientOcclusionUniform(const Terrain& terrain, const std::vector<HorizonAngles>& horizonAngles);

std::vector<float> ambientOcclusionDirectionalUniform(const Terrain& terrain, const std::vector<HorizonAngles>& horizonAngles);

/**
 * \brief Compute the coefficients of the texture storing the light map.
 * \return The coefficients of the light map.
 */
std::vector<float> computeLightMap(const Terrain& terrain, const std::vector<HorizonAngles>& horizonAngles, const Parameters& parameters);

}

#endif // OCCLUSION_H