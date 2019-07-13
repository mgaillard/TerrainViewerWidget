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

QImage TerrainViewer::normalTextureImage(const Terrain& terrain)
{
	const auto normalMap = computeNormals(terrain);

	QImage image(terrain.resolutionWidth(), terrain.resolutionHeight(), QImage::Format_RGB32);

#pragma omp parallel for
	for (int i = 0; i < terrain.resolutionHeight(); i++)
	{
		for (int j = 0; j < terrain.resolutionWidth(); j++)
		{
			const auto index = i * terrain.resolutionWidth() + j;

			const auto red = static_cast<uint8_t>(255.0f * (normalMap[index].x() + 1.0f) / 2.0f);
			const auto green = static_cast<uint8_t>(255.0f * (normalMap[index].y() + 1.0f) / 2.0f);
			const auto blue = static_cast<uint8_t>(255.0f * (normalMap[index].z() + 1.0f) / 2.0f);
			image.setPixel(j, i, qRgb(red, green, blue));
		}
	}

	return image;
}
