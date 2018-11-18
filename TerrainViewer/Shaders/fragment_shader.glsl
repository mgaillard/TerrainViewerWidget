#version 400

in vec3 position_world;
in vec3 normal_world;

out vec4 fragColor;

const vec3 ambient_color = vec3(0.95, 0.95, 0.95);
const vec3 diffuse_color = vec3(0.95, 0.95, 0.95);
const vec3 light_direction = normalize(vec3(1.0, 1.0, 1.0));
 
void main()
{
	vec3 ambient_term = 0.1 * ambient_color;
	vec3 diffuse_term = 0.9 * diffuse_color * max(0.0, dot(normal_world, light_direction));

	fragColor = vec4(ambient_term + diffuse_term, 1.0);
}