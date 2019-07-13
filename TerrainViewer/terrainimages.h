#ifndef TERRAINIMAGES_H
#define TERRAINIMAGES_H

#include <QVector4D>

#include "terrain.h"

namespace TerrainViewer
{

/**
 * \brief Compute the normals of the terrain on the CPU.
 * \return An array of 4D vectors. The fourth component is always 0.
 */
std::vector<QVector4D> computeNormals(const Terrain& terrain);

}

#endif // TERRAINIMAGES_H