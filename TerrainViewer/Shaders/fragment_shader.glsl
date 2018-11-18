#version 400

uniform float max_altitude;

in vec3 position_world;
in vec3 normal_world;

out vec4 fragColor;

vec3 shading_diffuse()
{
	vec3 ambient_color = vec3(0.95, 0.95, 0.95);
	vec3 diffuse_color = vec3(0.95, 0.95, 0.95);
	vec3 light_direction = normalize(vec3(1.0, 1.0, 1.0));

	vec3 ambient_term = 0.1 * ambient_color;
	vec3 diffuse_term = 0.9 * diffuse_color * max(0.0, dot(normal_world, light_direction));

	return ambient_term + diffuse_term;
}

vec3 shading_texture()
{
	// Normalized altitude
	float normalized_altitude = position_world.y / max_altitude;

	// Height shading: color is a linear interpolation of height colors
	vec3 color_altitude = mix(vec3(0.75, 0.725, 0.70), vec3(0.95, 0.925, 0.90), normalized_altitude);

	float lambertian = max(0.0, dot(normal_world, normalize(vec3(1.0, 2.5, 0.5))));
	lambertian = 0.5*(1.0 + lambertian); // Remap in [0, 1]
	lambertian = lambertian*lambertian;

	return color_altitude*(1.0 + lambertian)/2.0;
}

vec3 shading_guerin()
{
	// Normalized altitude
	float normalized_altitude = position_world.y / max_altitude;

	// Height shading: color is a linear interpolation of height colors
	vec3 color_altitude = mix(vec3(0.75, 0.725, 0.70), vec3(0.95, 0.925, 0.90), normalized_altitude);

	float lambertian = max(0.0, dot(normal_world, normalize(vec3(1.0, 2.5, 0.5))));
	lambertian = 0.5*(1.0 + lambertian); // Remap in [0, 1]
	lambertian = lambertian*lambertian;

	// Normalized direction
	float t = dot(normal_world.xz, normalize(vec2(1.0, 1.0)));
	t = 0.5*(1.0 + t); // Remap in [0, 1]
	vec3 color_normal = lambertian*mix(vec3(0.65, 0.75, 0.85), vec3(1.0, 0.95, 0.8), t);

	// GPUTerrainViewer version
	vec3 color = 0.1*vec3(0.95, 0.95, 0.95) + 0.8*color_normal + 0.1*color_altitude;

	// LibCore version
	// vec3 color = 0.25*vec3(0.975, 0.975, 0.975) + 0.50*color_normal + 0.25*color_altitude;

	return color;
}

void main()
{
	vec3 color = shading_texture();
	fragColor = vec4(color, 1.0);
}