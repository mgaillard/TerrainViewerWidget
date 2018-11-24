#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

uniform struct Terrain
{
	sampler2D texture;
	float height;
	float width;
	int resolution_height;
	int resolution_width;
	float max_altitude;
} terrain;

layout (quads, fractional_odd_spacing, ccw) in;

out vec3 position_world;
out vec3 normal_world;

float height(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain.width, p.y / terrain.height);
	return texture(terrain.texture, texcoord).r;
}

vec3 normal(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain.width, p.y / terrain.height);

	const float left = textureOffset(terrain.texture, texcoord, ivec2(-1, 0)).r;
	const float right = textureOffset(terrain.texture, texcoord, ivec2(1, 0)).r;
	const float bottom = textureOffset(terrain.texture, texcoord, ivec2(0, -1)).r;
	const float top = textureOffset(terrain.texture, texcoord, ivec2(0, 1)).r;

	const float step_width = terrain.width / (terrain.resolution_width - 1.0);
	const float step_height = terrain.height / (terrain.resolution_height - 1.0);

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
