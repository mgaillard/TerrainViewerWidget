#version 430

layout (local_size_x = 4, local_size_y = 4) in;

layout (r32f, binding = 0) uniform image2D heightmap;
layout (r32f, binding = 1) uniform image2D watermap;

// Out flow in the 4 directions, each chanel stores one direction.
//  - R = left
//  - G = right
//  - B = top
//  - A = bottom
layout (rgba32f, binding = 2) uniform image2D outflow;

uniform float terrain_height;
uniform float terrain_width;

uniform float time_step;
uniform float water_increment;
uniform float evaporation_rate;

uniform bool bounce_boundaries;

float height(const ivec2 coords)
{
	return imageLoad(heightmap, coords).r;
}

float water(const ivec2 coords)
{
	return imageLoad(watermap, coords).r;
}

float outFlowLeft(const ivec2 coords)
{
	return imageLoad(outflow, coords).r;
}

float outFlowRight(const ivec2 coords)
{
	return imageLoad(outflow, coords).g;
}

float outFlowTop(const ivec2 coords)
{
	return imageLoad(outflow, coords).b;
}

float outFlowBottom(const ivec2 coords)
{
	return imageLoad(outflow, coords).a;
}

float incrementWater(float current_water)
{
	return current_water + water_increment * time_step;
}

float evaporateWater(float current_water)
{
	return max(0.0, current_water - evaporation_rate * time_step);
}

float incrementAndEvaporateWater(float current_water)
{
	return evaporateWater(incrementWater(current_water));
}

// Simulate water on the heightmap: compute water height
void main()
{
	// Resolution of the maps
	const ivec2 origin = ivec2(0, 0);
	const ivec2 terrain_size = imageSize(heightmap);
	const ivec2 watermap_size = imageSize(watermap);
	const ivec2 outflow_size = imageSize(outflow);

	const float cell_width = terrain_width / terrain_size.x;
	const float cell_height = terrain_height / terrain_size.y;
	const float cell_area = cell_width * cell_height;

	// Coordinates of the cells on the terrain
	const ivec2 center_cell = ivec2(gl_GlobalInvocationID);
	const ivec2 left_cell = center_cell + ivec2(-1, 0);
	const ivec2 right_cell = center_cell + ivec2(1, 0);
	const ivec2 top_cell = center_cell + ivec2(0, -1);
	const ivec2 bottom_cell = center_cell + ivec2(0, 1);

	// If within the interior of the terrain
	if (all(greaterThanEqual(center_cell, ivec2(0, 0))) && all(lessThan(center_cell, terrain_size)))
	{

		// Water height in the center cell
		// We increment and evaporate again, even though it has been done in the last pass
		// We compute it in the flow pass but we don't save it because the result can be reused in the same pass
		float water_height_center = water(center_cell);
		water_height_center = incrementAndEvaporateWater(water_height_center);

		const float outflow_total = outFlowLeft(center_cell)
			                      + outFlowRight(center_cell)
			                      + outFlowTop(center_cell)
			                      + outFlowBottom(center_cell);

		// Compute the flow coming in this cell
		float inflow_total = 0.0;

		if (all(greaterThanEqual(left_cell, ivec2(0, 0))))
		{
			inflow_total += outFlowRight(left_cell);
		}

		if (all(lessThan(right_cell, terrain_size)))
		{
			inflow_total += outFlowLeft(right_cell);
		}
		
		if (all(greaterThanEqual(top_cell, ivec2(0, 0))))
		{
			inflow_total += outFlowBottom(top_cell);
		}

		if (all(lessThan(bottom_cell, terrain_size)))
		{
			inflow_total += outFlowTop(bottom_cell);
		}                    

		float new_water_height = max(0.0, water_height_center + ((inflow_total - outflow_total) * time_step) / cell_area);

		// Boundary conditions
		if (!bounce_boundaries)
		{
			// Water does not bounce on boundaries (remove water in cells adjacent to borders)
			if (center_cell.x <= 0
			 || center_cell.y <= 0
			 || center_cell.x >= terrain_size.x - 1
			 || center_cell.y >= terrain_size.y - 1)
			{
				new_water_height = 0.0;
			}
		}

		// Output the water height
		imageStore(watermap, center_cell, vec4(new_water_height));
	}
}