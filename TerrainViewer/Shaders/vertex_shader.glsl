#version 400
uniform mat4 PVM;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;

void main()
{
	gl_Position = PVM * vec4(pos_attrib, 1.0);
}