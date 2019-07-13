#include "terrainimages.h"

using namespace TerrainViewer;

std::vector<QVector4D> TerrainViewer::computeNormals(const Terrain& terrain)
{
	std::vector<QVector4D> normals(terrain.resolutionWidth() * terrain.resolutionHeight());

#pragma omp parallel for
	for (int i = 0; i < terrain.resolutionHeight(); i++)
	{
		for (int j = 0; j < terrain.resolutionWidth(); j++)
		{
			const int index = i * terrain.resolutionWidth() + j;

			const QVector3D normal = terrain.normal(i, j).normalized();
			normals[index].setX(normal.x());
			normals[index].setY(normal.y());
			normals[index].setZ(normal.z());
			normals[index].setW(0.0);
		}
	}

	return normals;
}
