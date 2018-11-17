#version 400
uniform mat4 PVM;

in vec3 pos_attrib;
in vec3 normal_attrib;

out vec3 normal_world;

void main()
{
	gl_Position = PVM * vec4(pos_attrib, 1.0);
	normal_world = normalize(normal_attrib);
}