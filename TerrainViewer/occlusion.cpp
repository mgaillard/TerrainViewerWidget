#include "occlusion.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "utils.h"

using namespace TerrainViewer;

/**
 * \brief Compute the tangent of the horizon angle between two cells on the terrain
 * \param i1 I coordinate of the current cell
 * \param j1 J coordinate of the current cell
 * \param h1 Altitude of the current cell
 * \param i2 I coordinate of the horizon cell
 * \param j2 J coordinate of the horizon cell
 * \param h2 Altitude of the horizon cell
 * \param cellWidth Width of a cell in the terrain
 * \param cellHeight Height of a cell in the terrain
 * \return The tangent of the horizon angle between two cells on the terrain
 */
float horizonAngleScanSlope(int i1, int j1, float h1, int i2, int j2, float h2, float cellWidth, float cellHeight)
{
	// Altitude difference
	const float dh = h2 - h1;
	// Distance between the two points
	const float d = std::sqrt(cellHeight * cellHeight * (i2 - i1) * (i2 - i1)
		+ cellWidth * cellWidth * (j2 - j1) * (j2 - j1));
	// Tangent of the angle
	return dh / d;
}

/**
 * \brief Compute the horizon angle in one cell of a terrain
 *        Brute force algorithm, mainly for testing purpose
 * \param terrain 
 * \param i I coordinate of the cell
 * \param j J coordinate of the cell
 * \param di I coordinate of the azimuthal direction
 * \param dj J coordinate of the azimuthal direction
 * \return The horizon angle
 */
float horizonAngleBruteForce(const TerrainViewer::Terrain& terrain, int i, int j, int di, int dj)
{
	const int height = terrain.resolutionHeight();
	const int width = terrain.resolutionWidth();

	const float cellWidth = terrain.cellWidth();
	const float cellHeight = terrain.cellHeight();

	// Horizon angle for this point
	float horizonTan = 0.0;

	// Compute the horizon in this point for a direction (dx, dy)
	for (int k = i + di, l = j + dj; k >= 0 && k < height && l >= 0 && l < width; k += di, l += dj)
	{
		// Horizon angle to this point
		const float tanAngle = horizonAngleScanSlope(i, j, terrain(i, j), k, l, terrain(k, l), cellWidth, cellHeight);

		horizonTan = std::max(tanAngle, horizonTan);
	}

	return M_PI_2 - std::atan(horizonTan);
}

struct HorizonAngles
{
	// Horizon angles directions
	static const std::array<std::pair<int, int>, 16> directions;

	// Array of 16 horizon angles value initialized
	std::array<float, 16> angles{};
};

const std::array<std::pair<int, int>, 16> HorizonAngles::directions = {
{
	{ 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 },   // 4-connected neighborhood
	{ 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 }, // Diagonals
	{ 2, 1 }, { 2, -1 }, { -2, 1 }, { -2, -1 }, // Knight in chess
	{ 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }
} };

/**
 * \brief Compute the horizon angles of every cell in one direction
 * Timonen, V., &Westerholm, J. (2010, May).Scalable Height Field Self‐Shadowing.
 * In Computer Graphics Forum(Vol. 29, No. 2, pp. 723 - 731).Oxford, UK: Blackwell Publishing Ltd.
 * http://wili.cc/research/hfshadow/hfshadow.pdf
 * Sean Barrett, 2011-12-25
 * http://nothings.org/gamedev/horizon/
 * Heman library on Github
 * https://github.com/prideout/heman
 * \param terrain A terrain
 * \param direction The index of the azimuthal direction in which the horizon angles are computed
 * \param horizonAngles An array in which the angles are stored
 */
void horizonAngleScan(const TerrainViewer::Terrain& terrain, int direction, std::vector<HorizonAngles>& horizonAngles)
{
	const int di = HorizonAngles::directions[direction].first;
	const int dj = HorizonAngles::directions[direction].second;

	const int width = terrain.resolutionWidth();
	const int height = terrain.resolutionHeight();

	const float cellWidth = terrain.cellWidth();
	const float cellHeight = terrain.cellHeight();

	const int si = sgn(di), sj = sgn(dj);
	const int ai = std::abs(di), aj = std::abs(dj);

	// Generate the start positions for each sweep line. The start positions occur just outside the image boundary.
	const int nbSweeps = ai * width + aj * height - (ai + aj - 1);

	std::vector<std::pair<int, int>> startPts;
	startPts.reserve(nbSweeps);
	for (int j = -aj; j < width - aj; j++)
	{
		for (int i = -ai; i < height - ai; i++)
		{
			if (j >= 0 && j < width && i >= 0 && i < height)
			{
				continue;
			}

			const int pi = (si < 0) ? (height - i - 1) : i;
			const int pj = (sj < 0) ? (width - j - 1) : j;
			startPts.emplace_back(pi, pj);
		}
	}

	assert(nbSweeps == static_cast<int>(startPts.size()));

	for (int sweep = 0; sweep < nbSweeps; sweep++)
	{
		// (i, j) is the current point
		int i = startPts[sweep].first;
		int j = startPts[sweep].second;

		std::vector<std::pair<int, int>> convexHullBuffer;
		// Add this point to the convex hull
		convexHullBuffer.emplace_back(i, j);

		i += di;
		j += dj;
		while (i >= 0 && i < height && j >= 0 && j < width)
		{
			// Find the horizon point on the temporary convex hull
			while (convexHullBuffer.size() > 1)
			{
				// Slope of the last element
				const int k = convexHullBuffer[convexHullBuffer.size() - 1].first;
				const int l = convexHullBuffer[convexHullBuffer.size() - 1].second;
				const float slopeLast = horizonAngleScanSlope(i, j, terrain.atClamp(i, j),
					k, l, terrain.atClamp(k, l), cellWidth, cellHeight);

				// Slope of the penultimate element
				const int m = convexHullBuffer[convexHullBuffer.size() - 2].first;
				const int n = convexHullBuffer[convexHullBuffer.size() - 2].second;
				const float slopePenultimate = horizonAngleScanSlope(i, j, terrain.atClamp(i, j),
					m, n, terrain.atClamp(m, n), cellWidth, cellHeight);

				if (slopeLast >= slopePenultimate)
				{
					break;
				}
				convexHullBuffer.pop_back();
			}
			// Horizon point for (i, j) is convexHullBuffer.back()
			// Update the horizon angle for this point
			// Slope of the last element
			// TODO improve this section
			const int k = convexHullBuffer.back().first;
			const int l = convexHullBuffer.back().second;
			float slopeLast = horizonAngleScanSlope(i, j, terrain.atClamp(i, j), k, l, terrain.atClamp(k, l), cellWidth, cellHeight);
			slopeLast = std::max(slopeLast, 0.f);
			horizonAngles[i * width + j].angles[direction] = M_PI_2 - std::atan(slopeLast);

			// We add the current point to the convexHullBuffer
			convexHullBuffer.emplace_back(i, j);

			i += di;
			j += dj;
		}
	}
}

/**
 * \brief Mainly for testing purpose, use computeHorizonAnglesFast instead.
 * \param terrain A terrain
 * \return The horizon angles in one cell of the terrain
 */
std::vector<HorizonAngles> computeHorizonAnglesBruteForce(const Terrain& terrain)
{
	const int width = terrain.resolutionWidth();
	const int height = terrain.resolutionHeight();

	std::vector<HorizonAngles> horizonAngles(height * width);

#pragma omp parallel for
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (unsigned int d = 0; d < HorizonAngles::directions.size(); d++)
			{
				// Angle between 0 and pi/2
				horizonAngles[i * width + j].angles[d] = horizonAngleBruteForce(terrain, i, j,
					HorizonAngles::directions[d].first,
					HorizonAngles::directions[d].second);
			}
		}
	}

	return horizonAngles;
}

/**
 * \brief Compute the horizon angles on a terrain with a fast algorithm.
 *		  See horizonAngleScan
 * \param terrain A terrain
 * \return The horizon angles in each cell of the terrain
 */
std::vector<HorizonAngles> computeHorizonAnglesFast(const Terrain& terrain)
{
	const int width = terrain.resolutionWidth();
	const int height = terrain.resolutionHeight();

	std::vector<HorizonAngles> horizonAngles(height * width);

#pragma omp parallel for
	for (int d = 0; d < HorizonAngles::directions.size(); d++)
	{
		horizonAngleScan(terrain, d, horizonAngles);
	}

	return horizonAngles;
}

/**
 * \brief Compute the occlusion of a terrain with a diffuse light
 *        For each cell in the terrain we compute the percentage of the surface of 
 *        the hemisphere that is accessible by uniform ambient light
 * \param horizonAngles Horizon angles of a terrain
 * \return The occlusion value for each cell of the terrain in a flat array
 */
std::vector<float> computeOcclusionUniform(const std::vector<HorizonAngles>& horizonAngles)
{
	// Number of azimuthal directions
	const int nbDirections = HorizonAngles::directions.size();

	// Compute the occlusion value in every cell according to the horizon angles
	std::vector<float> occlusion(horizonAngles.size(), 0.0f);

#pragma omp parallel for
	for (int i = 0; i < horizonAngles.size(); i++)
	{
		for (int d = 0; d < nbDirections; d++)
		{
			// Percentage of the surface of the hemisphere that is accessible by uniform ambient light.
			occlusion[i] += horizonAngles[i].angles[d] / (nbDirections * M_PI_2);
		}
	}

	return occlusion;
}

std::vector<float> TerrainViewer::ambientOcclusion(const Terrain& terrain)
{
	// Compute the horizon angles in every cell
	const std::vector<HorizonAngles> horizonAngles = computeHorizonAnglesFast(terrain);

	// Compute the occlusion value in every cell according to the horizon angles
	std::vector<float> occlusion = computeOcclusionUniform(horizonAngles);

	// Remap between 0 and 1.
	// TODO: Let the user choose the mapping
	const auto itMinMax = std::minmax_element(occlusion.begin(), occlusion.end());
	const float minimum = *itMinMax.first;
	const float maximum = *itMinMax.second; // Should be about 1.0
	for (unsigned int i = 0; i < occlusion.size(); i++)
	{
		occlusion[i] = (occlusion[i] - minimum) / (maximum - minimum);
	}

	return occlusion;
}

/*
 * WIP: Lighting model
 *
const float intensity = 1.0;

float light = 0.0;

for (const auto& direction : directions)
{
	const QVector3D normal = terrain.normal(i, j).normalized();

	// Angle between 0 and pi/2
	const float angleZenith = horizonAngleBruteForce(terrain, i, j, direction.first, direction.second);

	// cos(2kpi/n)
	const float cosine = direction.second / std::hypot(direction.first, direction.second);
	// sin(2kpi/n)
	const float sine = direction.first / std::hypot(direction.first, direction.second);
	// nx * cos(2kpi/n) + ny * sin(2kpi/n)
	const float normalProjected = normal.x() * cosine + normal.y() * sine;
	// Clamp the angleZenith with the normal so that dot(N, e) >= 0
	const float theta = std::min(angleZenith, float(M_PI_2) + atan(normalProjected / normal.z()));

	light += intensity * (normal.z() / nbDirs) * sin(theta) * sin(theta);
	light += intensity * (sin(M_PI / nbDirs) / M_PI) * ((theta - 0.5*sin(2.0 * theta))*normalProjected);
}

occlusion[i * width + j] = light;
*/