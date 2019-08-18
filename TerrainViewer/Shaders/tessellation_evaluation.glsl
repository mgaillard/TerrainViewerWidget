#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

uniform struct Terrain
{
	sampler2D height_texture;
	sampler2D normal_texture;
	sampler2D lightMap_texture;
	sampler2D waterMap_texture;
	float height;
	float width;
	int resolution_height;
	int resolution_width;
	float max_altitude;
} terrain;

layout (quads, fractional_odd_spacing, ccw) in;

out vec3 position_model;
out vec3 position_world;
out vec3 normal_world;

float height(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain.width, p.y / terrain.height);
	const float terrain_height = texture(terrain.height_texture, texcoord).s;
	const float water_height = texture(terrain.waterMap_texture, texcoord).s;
	return terrain_height + water_height;
}

vec3 normal(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain.width, p.y / terrain.height);
	return normalize(texture(terrain.normal_texture, texcoord).stp);
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

	position_model = vec3(p);
	position_world = vec3(M * p);
	normal_world = normalize(N * normal(p.xy));
}
