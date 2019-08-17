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
const int PaletteEnvironment = 3;
uniform int palette = PaletteWhite;

// Shading method
const int ShadingNormal = 0;
const int ShadingUniformLightBasic = 1;
const int ShadingUniformLight = 2;
const int ShadingDirectionalLight = 3;
const int ShadingSlope = 4;
uniform int shading = ShadingNormal;

// Varying variables
in vec3 position_model;
in vec3 position_world;
in vec3 normal_world;

// Output
out vec4 fragColor;

float colormap_jet_red(float x) {
    if (x < 0.7) {
        return 4.0 * x - 1.5;
    } else {
        return -4.0 * x + 4.5;
    }
}

float colormap_jet_green(float x) {
    if (x < 0.5) {
        return 4.0 * x - 0.5;
    } else {
        return -4.0 * x + 3.5;
    }
}

float colormap_jet_blue(float x) {
    if (x < 0.3) {
       return 4.0 * x + 0.5;
    } else {
       return -4.0 * x + 2.5;
    }
}

// Matlab jet colormap_blue
// Source: https://github.com/kbinani/colormap-shaders
vec4 colormap_jet(float x) {
    float r = clamp(colormap_jet_red(x), 0.0, 1.0);
    float g = clamp(colormap_jet_green(x), 0.0, 1.0);
    float b = clamp(colormap_jet_blue(x), 0.0, 1.0);
    return vec4(r, g, b, 1.0);
}

// Elevation color ramp with t in [0, 1]
vec3 elevation_ramp_dem_screen(const float t)
{
	// Gradient name: "DEM screen"
	// Source: http://soliton.vm.bytemark.co.uk/pub/cpt-city/views/totp-svg.html
	const float offset[6] = float[](
		0.0, 0.125, 0.25, 0.5, 0.75, 1.0
	);
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

// Elevation color ramp with t in [0, 1]
vec3 elevation_ramp_europe(const float t)
{
	// Gradient name: "Europe 7"
	// Source: http://soliton.vm.bytemark.co.uk/pub/cpt-city/esri/hypsometry/eu/europe_7.svg
	const float offset[63] = float[](
		0.0000, 0.0238, 0.0397, 0.0556, 0.0714, 0.0873, 0.1032, 0.1190, 0.1349, 0.1508, 
		0.1667, 0.1825, 0.1984, 0.2143, 0.2302, 0.2460, 0.2619, 0.2778, 0.2937, 0.3095, 
		0.3254, 0.3413, 0.3571, 0.3730, 0.3889, 0.4048, 0.4206, 0.4365, 0.4524, 0.4683, 
		0.4841, 0.5000, 0.5159, 0.5317, 0.5476, 0.5635, 0.5794, 0.5952, 0.6111, 0.6270, 
		0.6429, 0.6587, 0.6746, 0.6905, 0.7063, 0.7222, 0.7381, 0.7540, 0.7698, 0.7857, 
		0.8016, 0.8175, 0.8333, 0.8492, 0.8651, 0.8810, 0.8968, 0.9127, 0.9286, 0.9444, 
		0.9603, 0.9762, 1.0000
	);
	const vec3 color[63] = vec3[](
		vec3(71,105,0) / 255.0, vec3(76,107,2) / 255.0, vec3(81,112,3) / 255.0, vec3(86,117,6) / 255.0, 
		vec3(92,122,7) / 255.0, vec3(98,128,10) / 255.0, vec3(102,133,12) / 255.0, vec3(109,138,14) / 255.0, 
		vec3(111,140,17) / 255.0, vec3(118,145,19) / 255.0, vec3(127,153,23) / 255.0, vec3(129,156,25) / 255.0, 
		vec3(136,161,29) / 255.0, vec3(141,166,31) / 255.0, vec3(148,171,34) / 255.0, vec3(155,176,39) / 255.0, 
		vec3(160,181,43) / 255.0, vec3(168,186,47) / 255.0, vec3(170,189,49) / 255.0, vec3(178,194,54) / 255.0, 
		vec3(187,201,60) / 255.0, vec3(190,204,63) / 255.0, vec3(197,209,67) / 255.0, vec3(205,214,73) / 255.0, 
		vec3(210,219,77) / 255.0, vec3(217,224,83) / 255.0, vec3(222,230,87) / 255.0, vec3(230,235,94) / 255.0, 
		vec3(237,240,98) / 255.0, vec3(242,245,105) / 255.0, vec3(250,250,110) / 255.0, vec3(255,255,115) / 255.0, 
		vec3(252,250,114) / 255.0, vec3(247,243,109) / 255.0, vec3(242,233,104) / 255.0, vec3(240,228,101) / 255.0, 
		vec3(237,223,100) / 255.0, vec3(232,216,95) / 255.0, vec3(230,211,92) / 255.0, vec3(224,202,88) / 255.0, 
		vec3(222,197,84) / 255.0, vec3(217,189,80) / 255.0, vec3(214,185,79) / 255.0, vec3(209,178,75) / 255.0, 
		vec3(207,173,72) / 255.0, vec3(204,166,69) / 255.0, vec3(199,159,66) / 255.0, vec3(196,154,63) / 255.0, 
		vec3(191,147,59) / 255.0, vec3(189,143,58) / 255.0, vec3(184,136,55) / 255.0, vec3(181,132,53) / 255.0, 
		vec3(179,125,50) / 255.0, vec3(173,119,47) / 255.0, vec3(168,112,44) / 255.0, vec3(166,109,43) / 255.0, 
		vec3(163,104,41) / 255.0, vec3(158,98,38) / 255.0, vec3(153,90,35) / 255.0, vec3(150,86,33) / 255.0, 
		vec3(148,82,31) / 255.0, vec3(143,77,30) / 255.0, vec3(140,73,28) / 255.0
	);

	for (int i = 0; i < 62; i++)
	{
		if (t >= offset[i] && t < offset[i + 1])
		{
			const float s = (t - offset[i]) / (offset[i + 1] - offset[i]);
			return mix(color[i], color[i + 1], s);
		}
	}

	return color[62];
}

// Elevation color ramp with t in [0, 1]
vec3 elevation_ramp_speed(const float t)
{
	// Gradient name: "Speed"
	// Source: http://soliton.vm.bytemark.co.uk/pub/cpt-city/cmocean/speed.svg
	const float offset[8] = float[](
		0.000, 0.143, 0.286, 0.429,
		0.571, 0.714, 0.857, 1.000
	);
	const vec3 color[8] = vec3[](
		vec3(201, 186, 69) / 255.0, vec3(178, 175, 40) / 255.0, vec3(152, 165, 18) / 255.0, vec3(124, 156, 6) / 255.0,
		vec3(95, 146, 12) / 255.0, vec3(66, 135, 25) / 255.0, vec3(39, 123, 35) / 255.0, vec3(17, 109, 42) / 255.0
	);

	for (int i = 0; i < 7; i++)
	{
		if (t >= offset[i] && t < offset[i + 1])
		{
			const float s = (t - offset[i]) / (offset[i + 1] - offset[i]);
			return mix(color[i], color[i + 1], s);
		}
	}

	return color[7];
}

vec3 compute_color_environment(const float altitude, const float slope, const float light)
{
	const vec3 snow_color = vec3(0.95, 0.95, 0.95);
	const vec3 mountain_rock_color = vec3(0.5, 0.5, 0.5);
	const vec3 plane_rock_color = vec3(0.5, 0.5, 0.5);
	const vec3 plane_dirt_color = vec3(0.608, 0.463, 0.325);

	// If altitude is more than 0.75 => mountain
	const float mountain_altitude_factor = smoothstep(0.70, 0.80, altitude);
	// If slope is less than 0.25 => snow
	const float snow_slope_factor = 1.0 - smoothstep(0.25, 0.45, slope);
	// Color in the mountain region
	const vec3 mountain_color = mix(mountain_rock_color, snow_color, snow_slope_factor);

	// If altitude is less than 0.75 => plane
	const float plane_altitude_factor = 1.0 - mountain_altitude_factor;
	// If slope is less than 0.20 => grass
	const float grass_slope_factor = 1.0 - smoothstep(0.20, 0.30, slope);
	// If slope is more than 0.20 => dirt + rock
	const float dirt_rock_slope_factor = smoothstep(0.45, 0.55, slope);
	const vec3 plane_dirt_rock_color = mix(plane_dirt_color, plane_rock_color, dirt_rock_slope_factor);
	const vec3 plane_grass_color = elevation_ramp_speed(altitude);
	// Color in the plane region
	const vec3 plane_color = mix(plane_dirt_rock_color, plane_grass_color, grass_slope_factor);

	return plane_altitude_factor * plane_color
	     + mountain_altitude_factor * mountain_color;
}

float compute_normalized_altitude()
{
	return position_model.z / terrain.max_altitude;
}

// Retun the slope, between 0.0 and 1.0
float compute_slope()
{
	return 1.0 - normal_world.z;
}

float compute_occlusion()
{
	const vec2 texcoord = vec2(position_model.x / terrain.width, position_model.y / terrain.height);
	return texture(terrain.lightMap_texture, texcoord).s;
}

// Compute the color of the fragment by choosing
// the right color palette
vec3 compute_color(const float altitude, const float slope, const float light)
{
	switch(palette)
	{
	case PaletteWhite:
		return vec3(0.95);
		break;

	case PaletteDemScreen:
		return elevation_ramp_dem_screen(altitude);
		break;

	case PaletteEnvironment:
		return compute_color_environment(altitude, slope, light);
		break;
	}

	// Default color
	return vec3(0.0);
}

vec3 shading_normal()
{
	const vec3 light_direction = normalize(vec3(1.0, 1.0, 1.0));

	// Normalized altitude
	const float normalized_altitude = compute_normalized_altitude();
	const float slope = compute_slope();
	const float normal_term = max(0.0, dot(normal_world, light_direction));
	const vec3 color = compute_color(normalized_altitude, slope, normal_term);

	return color * (0.25  + 0.75 * normal_term);
}

vec3 shading_slope()
{
	// Slope, between 0.0 and 1.0
	const float slope = compute_slope();

	return vec3(colormap_jet(slope));
}

vec3 shading_occlusion()
{
	// Normalized altitude
	const float normalized_altitude = compute_normalized_altitude();
	const float slope = compute_slope();
	const float occlusion = compute_occlusion();
	const vec3 color = compute_color(normalized_altitude, slope, occlusion);

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

	case ShadingUniformLightBasic:
	case ShadingUniformLight:
	case ShadingDirectionalLight:
		return shading_occlusion();
	break;

	case ShadingSlope:
		return shading_slope();
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
