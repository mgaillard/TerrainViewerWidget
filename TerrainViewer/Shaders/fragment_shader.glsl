#version 430

uniform struct Terrain
{
	sampler2D height_texture;
	sampler2D normal_texture;
	sampler2D lightMap_texture;
	float height;
	float width;
	int resolution_height;
	int resolution_width;
	float max_altitude;
} terrain;

// Color palette
const int PaletteWhite = 1;
const int PaletteDemScreen = 2;
uniform int palette = PaletteWhite;

// Shading method
const int ShadingNormal = 0;
const int ShadingUniformAmbientOcclusion = 1;
uniform int shading = ShadingNormal;

// Varying variables
in vec3 position_model;
in vec3 position_world;
in vec3 normal_world;

// Output
out vec4 fragColor;

// Elevation color ramp with t in [0, 1]
vec3 elevation_ramp(const float t)
{
	const float offset[6] = float[](
		0.0,
		0.125,
		0.25,
		0.5,
		0.75,
		1.0
	);
	
	// Gradient name: "DEM screen"
	// Source: http://soliton.vm.bytemark.co.uk/pub/cpt-city/views/totp-svg.html
	const vec3 color[6] = vec3[](
		vec3(0, 132, 53) / 255.0,
		vec3(51, 204, 0) / 255.0,
		vec3(244, 240, 113) / 255.0,
		vec3(244, 189, 69) / 255.0,
		vec3(153, 100, 43) / 255.0,
		vec3(255, 255, 255) / 255.0
	);
	
	for (int i = 0; i < 5; i++)
	{
		if (t >= offset[i] && t < offset[i + 1])
		{
			const float s = (t - offset[i]) / (offset[i + 1] - offset[i]);
			return mix(color[i], color[i + 1], s);
		}
	}

	return color[5];
}

// Compute the color of the fragment by choosing
// the right color palette
vec3 compute_color(float altitude)
{
	switch(palette)
	{
	case PaletteWhite:
		return vec3(0.95);
	break;

	case PaletteDemScreen:
		return elevation_ramp(altitude);
	break;
	}

	// Default color
	return vec3(0.0);
}

vec3 shading_normal()
{
	const vec3 light_direction = normalize(vec3(1.0, 1.0, 1.0));

	// Normalized altitude
	const float normalized_altitude = position_model.z / terrain.max_altitude;
	const vec3 color = compute_color(normalized_altitude);

	const float normal_term = max(0.0, dot(normal_world, light_direction));

	return color * (0.25  + 0.75 * normal_term);
}

vec3 shading_occlusion()
{
	// Normalized altitude
	const float normalized_altitude = position_model.z / terrain.max_altitude;
	const vec3 color = compute_color(normalized_altitude);

	// Occlusion
	const vec2 texcoord = vec2(position_model.x / terrain.width, position_model.y / terrain.height);
	const float occlusion = texture(terrain.lightMap_texture, texcoord).s;

	return color * occlusion;
}

// Compute the shading of the fragment by choosing
// the right shading function
vec3 compute_shading()
{
	switch(shading)
	{
	case ShadingNormal:
		return shading_normal();
	break;

	case ShadingUniformAmbientOcclusion:
		return shading_occlusion();
	break;
	}

	// Default shading
	return vec3(0.0);
}

void main()
{
	vec3 color = compute_shading();
	fragColor = vec4(color, 1.0);
}
