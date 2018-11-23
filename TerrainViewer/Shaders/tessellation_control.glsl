#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

uniform vec2 viewportSize;

uniform float pixelsPerTriangleEdge = 16.0;

layout(vertices = 4) out;  //number of output verts of the tess. control shader

const float maxTessellationLevel = 64.0;

// Transform an object-space vertex in world-space coordinates
vec4 world(const vec4 vertex)
{
	return M * vertex;
}

// Project a vertex in world-space coordinates
vec4 project(const vec4 vertex)
{
	const vec4 result = PV * vertex;
	return result / result.w;
}

// Return true is the vertex (clip-space) is off the screen, false otherwise
bool off_screen(const vec4 vertex)
{
	return (vertex.z < -0.5   // The vertex is behind the camera
	     || vertex.x < -1.7   // The vertex is too far on the left
		 || vertex.x >  1.7   // The vertex is too far on the right
		 || vertex.y < -1.7   // The vertex is too far on the bottom
		 || vertex.y >  1.7); // The vertex is too far on the top
}

// The sphere diameter in clip space heuristic
// https://developer.nvidia.com/content/dynamic-hardware-tessellation-basics
float calc_tessellation_level(const vec4 v1, const vec4 v2)
{
	const float diameter = distance(v1, v2);
	const vec4 mid_point = mix(v1, v2, 0.5);
	const vec4 mid_point_clip = PV * mid_point;
	
	// Diameter of the sphere on the screen
	const float d = abs(diameter * P[1][1] / mid_point_clip.w);

	// Maximum between height and width of the screen
	const float max_screen = max(viewportSize.x, viewportSize.y);

    return clamp(d * max_screen / pixelsPerTriangleEdge, 1.0, maxTessellationLevel);
}

void main()
{
	if (gl_InvocationID == 0)
	{
		// The 4 corners of the patch in world-space coordinates
		const vec4 p0_world = world(gl_in[0].gl_Position);
		const vec4 p1_world = world(gl_in[1].gl_Position);
		const vec4 p2_world = world(gl_in[2].gl_Position);
		const vec4 p3_world = world(gl_in[3].gl_Position);

		// The 4 corners of the patch in clip-space coordinates
		const vec4 p0_clip = project(p0_world);
		const vec4 p1_clip = project(p1_world);
		const vec4 p2_clip = project(p2_world);
		const vec4 p3_clip = project(p3_world);

		// All corners of the patch are outside the screen
		if (off_screen(p0_clip) && off_screen(p1_clip) && off_screen(p2_clip) && off_screen(p3_clip))
		{
			// The patch is culled
			gl_TessLevelOuter[0] = 0;
			gl_TessLevelOuter[1] = 0;
			gl_TessLevelOuter[2] = 0;
			gl_TessLevelOuter[3] = 0;
			gl_TessLevelInner[0] = 0;
			gl_TessLevelInner[1] = 0;
		}
		else
		{
			gl_TessLevelOuter[0] = calc_tessellation_level(p3_world, p0_world);
			gl_TessLevelOuter[1] = calc_tessellation_level(p0_world, p1_world);
			gl_TessLevelOuter[2] = calc_tessellation_level(p1_world, p2_world);
			gl_TessLevelOuter[3] = calc_tessellation_level(p2_world, p3_world);
			gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[2], 0.5);
			gl_TessLevelInner[1] = mix(gl_TessLevelOuter[1], gl_TessLevelOuter[3], 0.5);
		}
	}
	
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
