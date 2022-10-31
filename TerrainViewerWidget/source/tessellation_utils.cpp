#include "tessellation_utils.h"

using namespace TerrainViewer;

std::vector<TessellationPatch> TerrainViewer::generateTessellationPatches(
	float height,
	float width,
	int numberPatchesHeight,
	int numberPatchesWidth)
{
	const auto numberPatches = numberPatchesHeight * numberPatchesWidth;
	const auto patchSizeHeight = height / float(numberPatchesHeight);
	const auto patchSizeWidth = width / float(numberPatchesWidth);

	std::vector<TessellationPatch> patches;
	patches.reserve(numberPatches);
	for (auto i = 0; i < numberPatchesHeight; i++)
	{
		for (auto j = 0; j < numberPatchesWidth; j++)
		{
			const auto x = patchSizeHeight * i;
			const auto y = patchSizeWidth * j;

			patches.emplace_back(x, y, patchSizeHeight, patchSizeWidth);
		}
	}

	return patches;
}
