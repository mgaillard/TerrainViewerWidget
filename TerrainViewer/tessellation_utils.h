#ifndef TESSELLATIONUTILS_H
#define TESSELLATIONUTILS_H

#include <vector>

#include <QVector3D>

namespace TerrainViewer
{

/**
 * \brief A struct to store a tessellation patch sent to OpenGL.
 */
struct TessellationPatch
{
	QVector3D v[4];

	TessellationPatch(const float x, const float y, const float sizeX, const float sizeY)
	{
		v[0] = { x        , y        , 0.0 };
		v[1] = { x + sizeX, y        , 0.0 };
		v[2] = { x + sizeX, y + sizeY, 0.0 };
		v[3] = { x        , y + sizeY, 0.0 };
	}
};

/**
 * \brief Generate patches for the tessellation shader.
 * \param height Height of the terrain.
 * \param width Width of the terrain.
 * \param numberPatchesHeight Number of patches in the height axis.
 * \param numberPatchesWidth Number of patches in the width axis.
 * \return A vector of patches for the tessellation shader.
 */
std::vector<TessellationPatch> generateTessellationPatches(float height,
														   float width,
														   int numberPatchesHeight,
														   int numberPatchesWidth);

}

#endif // TESSELLATIONUTILS_H
