#ifndef OCCLUSION_H
#define OCCLUSION_H

#include <vector>

#include "terrain.h"

namespace TerrainViewer
{

std::vector<float> ambientOcclusion(const TerrainViewer::Terrain& terrain);

}

#endif // OCCLUSION_H