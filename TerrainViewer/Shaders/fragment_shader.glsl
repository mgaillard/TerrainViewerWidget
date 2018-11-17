#version 400

in vec3 normal_world;

out vec4 fragColor;
 
void main()
{
	vec3 diffuse_color = vec3(0.0, 1.0, 0.0);
	vec3 light_world = vec3(0.0, 1.0, 0.0);

	vec3 ambient_term = vec3(0.0, 0.5, 0.0);
	vec3 diffuse_term = diffuse_color * max(0.0, dot(normal_world, light_world));

	fragColor = vec4(normal_world, 1.0);

	// fragColor = vec4(ambient_term + diffuse_term, 1.0);
}