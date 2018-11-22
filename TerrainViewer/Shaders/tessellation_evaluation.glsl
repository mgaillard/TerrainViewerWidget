#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform float terrain_height;
uniform float terrain_width;
uniform float terrain_max_altitude;

uniform sampler2D terrain;

layout (quads, equal_spacing, ccw) in;

float height(const vec2 p)
{
	const vec2 texcoord = vec2(p.x / terrain_height, p.y / terrain_width);
	return texture(terrain, texcoord);
}

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;
	vec4 p3 = gl_in[3].gl_Position;

	vec4 p = (1.0-u)*(1.0-v)*p0 + u*(1.0-v)*p1 + u*v*p2 + (1.0-u)*v*p3;

	// Apply terrain height offset in z-direction
	p.z = height(vec2(p));

	gl_Position = P*V*M*p;
}
