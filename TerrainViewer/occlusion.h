#ifndef OCCLUSION_H
#define OCCLUSION_H

#include <vector>

#include "terrain.h"

namespace TerrainViewer
{

std::vector<float> ambientOcclusionBasic(const TerrainViewer::Terrain& terrain);

std::vector<float> ambientOcclusionUniform(const TerrainViewer::Terrain& terrain);

}

#endif // OCCLUSION_H