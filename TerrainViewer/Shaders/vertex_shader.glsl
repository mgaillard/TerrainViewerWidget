#version 400

uniform mat4 PVM;
uniform mat4 M;
uniform mat3 N;

in vec3 pos_attrib;
in vec3 normal_attrib;

out vec3 position_world;
out vec3 normal_world;

void main()
{
	gl_Position = PVM * vec4(pos_attrib, 1.0);

	//Compute world-space vertex position
	position_world = vec3(M * vec4(pos_attrib, 1.0));

	// Compute world-space normal position
	normal_world = normalize(N * normal_attrib);
}