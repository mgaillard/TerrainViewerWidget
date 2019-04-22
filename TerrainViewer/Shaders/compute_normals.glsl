#version 430

layout (local_size_x = 4, local_size_y = 4) in;

layout (r32f, binding = 0) uniform image2D heightmap;
layout (rgba32f, binding = 1) uniform image2D normals;

uniform float terrain_height;
uniform float terrain_width;

// Compute the normals in heightmap and output the result in normals
void main()
{
	// Resolution of the terrain
	const ivec2 terrainSize = imageSize(heightmap);
	const ivec2 normalMapSize = imageSize(normals);

	// Coordinates on the terrain
	const ivec2 coords = ivec2(gl_GlobalInvocationID);

	// Compute the normal according to the heightmap
	vec3 normal = vec3(0.0, 0.0, 1.0);
	
	// If within the interior of the terrain
	if (all(greaterThan(coords, ivec2(0, 0))) && all(lessThan(coords, terrainSize - ivec2(1, 1))))
	{
		const float top = imageLoad(heightmap, coords + ivec2(0, -1)).r;
		const float bottom = imageLoad(heightmap, coords + ivec2(0, 1)).r;
		const float left = imageLoad(heightmap, coords + ivec2(-1, 0)).r;
		const float right = imageLoad(heightmap, coords + ivec2(1, 0)).r;

		const float stepX = terrain_width / (terrainSize.x - 1);
		const float stepY = terrain_height / (terrainSize.y - 1);

		const float xDiff = (right - left) / (2.0 * stepX);
		const float yDiff = (bottom - top) / (2.0 * stepY);

		normal = vec3(-xDiff, -yDiff, 1.0);
	}

	// Output the normal vector if within the image
	if (all(lessThanEqual(coords, normalMapSize)))
	{
		imageStore(normals, coords, vec4(normal, 1.0));
	}
}