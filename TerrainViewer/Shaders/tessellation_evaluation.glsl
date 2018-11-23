#version 430

layout (quads, equal_spacing, ccw) in;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

uniform float terrain_height;
uniform float terrain_width;
uniform int terrain_resolution_height;
uniform int terrain_resolution_width;
uniform float terrain_max_altitude;

uniform sampler2D terrain;

out vec3 position_world;
out vec3 normal_world;

float height(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain_width, p.y / terrain_height);
	return texture(terrain, texcoord).r;
}

vec3 normal(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain_width, p.y / terrain_height);

	const float left = textureOffset(terrain, texcoord, ivec2(-1, 0)).r;
	const float right = textureOffset(terrain, texcoord, ivec2(1, 0)).r;
	const float bottom = textureOffset(terrain, texcoord, ivec2(0, -1)).r;
	const float top = textureOffset(terrain, texcoord, ivec2(0, 1)).r;

	const float step_width = terrain_width / (terrain_resolution_width - 1.0);
	const float step_height = terrain_height / (terrain_resolution_height - 1.0);

	const float dhdx = (right - left) / (2.0 * step_width);
	const float dhdy = (top - bottom) / (2.0 * step_height);

	return normalize(vec3(-dhdx, -dhdy, 1.0));
}

void main()
{
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;
	const vec4 p2 = gl_in[2].gl_Position;
	const vec4 p3 = gl_in[3].gl_Position;

	const vec4 a = mix(p0, p1, u);
	const vec4 b = mix(p3, p2, u);

	vec4 p = mix(a, b, v);

	// Apply terrain height offset in z-direction
	p.z = height(p.xy);

	gl_Position = PVM * p;

	position_world = vec3(M * p);
	normal_world = normalize(N * normal(p.xy));
}
