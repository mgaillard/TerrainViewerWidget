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

// Simulate water on the heightmap: compute out flow
void main()
{
	// Resolution of the maps
	const ivec2 origin = ivec2(0, 0);
	const ivec2 terrain_size = imageSize(heightmap);
	const ivec2 watermap_size = imageSize(watermap);
	const ivec2 outflow_size = imageSize(outflow);

	const float cell_width = terrain_width / terrain_size.x;
	const float cell_height = terrain_height / terrain_size.y;

	// Coordinates of the cells on the terrain
	const ivec2 center_cell = ivec2(gl_GlobalInvocationID);
	const ivec2 left_cell = clamp(center_cell + ivec2(-1, 0), origin, terrain_size - ivec2(1, 1));
	const ivec2 right_cell = clamp(center_cell + ivec2(1, 0), origin, terrain_size - ivec2(1, 1));
	const ivec2 top_cell = clamp(center_cell + ivec2(0, -1), origin, terrain_size - ivec2(1, 1));
	const ivec2 bottom_cell = clamp(center_cell + ivec2(0, 1), origin, terrain_size - ivec2(1, 1));

	// If within the interior of the terrain
	if (all(greaterThanEqual(center_cell, ivec2(0, 0))) && all(lessThan(center_cell, terrain_size)))
	{
		// Water height in the 4 neighbors
		float water_height_center = water(center_cell);
		float water_height_left = water(left_cell);
		float water_height_right = water(right_cell);
		float water_height_top = water(top_cell);
		float water_height_bottom = water(bottom_cell);

		// Increment and evaporate water
		water_height_center = incrementAndEvaporateWater(water_height_center);
		water_height_left = incrementAndEvaporateWater(water_height_left);
		water_height_right = incrementAndEvaporateWater(water_height_right);
		water_height_top = incrementAndEvaporateWater(water_height_top);
		water_height_bottom = incrementAndEvaporateWater(water_height_bottom);

		// Altitude of the 4 neighbors
		const float altitude_center = height(center_cell);
		const float altitude_left = height(left_cell);
		const float altitude_right = height(right_cell);
		const float altitude_top = height(top_cell);
		const float altitude_bottom = height(bottom_cell);

		// Difference in height
		const float diff_left = (altitude_center + water_height_center) - (altitude_left + water_height_left);
		const float diff_right = (altitude_center + water_height_center) - (altitude_right + water_height_right);
		const float diff_top = (altitude_center + water_height_center) - (altitude_top + water_height_top);
		const float diff_bottom = (altitude_center + water_height_center) - (altitude_bottom + water_height_bottom);

		// Compute change in flow
		const float flow_left = max(0.0, outFlowLeft(center_cell) + time_step * diff_left / cell_width);
		const float flow_right = max(0.0, outFlowRight(center_cell) + time_step * diff_right / cell_width);
		const float flow_top = max(0.0, outFlowTop(center_cell) + time_step * diff_top / cell_height);
		const float flow_bottom = max(0.0, outFlowBottom(center_cell) + time_step * diff_bottom / cell_height);

		const float flow_sum = flow_left + flow_right + flow_top + flow_bottom;

		// The new out flow for this cell, by default 0.0
		vec4 new_outflow = vec4(0.0, 0.0, 0.0, 0.0);

		// If water is flowing through this cell, update the out flow
		if (flow_sum > 0.0)
		{
			// If the sum of the flow is greater than the quantity of water in the cell, flow is scaled down
			const float K = clamp((water_height_center * cell_width * cell_height) / (flow_sum * time_step), 0.0, 1.0);
			new_outflow = vec4(flow_left, flow_right, flow_top, flow_bottom) * K;
		}

		// Boundary conditions
		if (!bounce_boundaries)
		{
			// Water does not bounce on boundaries (no flow to the borders)

			if (center_cell.x <= 0)
			{
				// If on the left border of the terrain, the left flow is null
				new_outflow.r = 0.0;
			}
			if (center_cell.y <= 0)
			{
				// If on the top border of the terrain, the top flow is null
				new_outflow.b = 0.0;
			}
			if (center_cell.x >= terrain_size.x - 1)
			{
				// If on the right border of the terrain, the right flow is null
				new_outflow.g = 0.0;
			}
			if (center_cell.y >= terrain_size.y - 1)
			{
				// If on the bottom border of the terrain, the bottom flow is null
				new_outflow.a = 0.0;
			}
		}

		// Output the flow vector
		imageStore(outflow, center_cell, new_outflow);
	}
}