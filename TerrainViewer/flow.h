#ifndef FLOW_H
#define FLOW_H

#include <vector>

#include "terrain.h"

namespace TerrainViewer
{

std::vector<float> computeFlowMap(const Terrain& terrain);

}

#endif // FLOW_H