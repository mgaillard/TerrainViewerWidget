#ifndef TERRAINIMAGES_H
#define TERRAINIMAGES_H

#include <QVector4D>
#include <QImage>

#include "terrain.h"
#include "terrainviewerparameters.h"

namespace TerrainViewer
{

/**
 * \brief Compute the normals of the terrain on the CPU.
 * \return An array of 4D vectors. The fourth component is always 0.
 */
std::vector<QVector4D> computeNormals(const Terrain& terrain);

/**
 * \brief Return an image of the normal texture.
 * \return A 8 bits RGB image of the normal texture.
 */
QImage normalTextureImage(const Terrain& terrain);

/**
 * \brief Return an image of the light map texture.
 * \return A 8 bits grayscale image of the light map texture.
 */
QImage lightMapTextureImage(const Terrain& terrain, const Parameters& parameters);

/**
 * \brief Return an image of the terrain texture with lighting.
 * \return A 8 bits color image of the terrain texture.
 */
QImage demTextureImage(const Terrain& terrain, const Parameters& parameters);

}

#endif // TERRAINIMAGES_H