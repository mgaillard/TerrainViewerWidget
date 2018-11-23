#version 430

uniform float terrain_height;
uniform float terrain_width;
uniform int terrain_resolution_height;
uniform int terrain_resolution_width;
uniform float terrain_max_altitude;

uniform sampler2D terrain;

in vec3 pos_attrib;

float height(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain_width, p.y / terrain_height);
	return texture(terrain, texcoord).r;
}

void main()
{
	const float z = height(pos_attrib.xy);
	gl_Position = vec4(pos_attrib.xy, z, 1.0);
}