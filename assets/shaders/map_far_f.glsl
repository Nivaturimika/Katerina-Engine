in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D provinces_texture_sampler;
uniform sampler2D terrain_texture_sampler;
uniform sampler2DArray terrainsheet_texture_sampler;
uniform sampler2D overlay;
uniform sampler2DArray province_color;
uniform sampler2D colormap_political;
uniform sampler2D province_highlight;
uniform sampler2D stripes_texture;
uniform sampler2D colormap_water;

// location 0 : offset
// location 1 : zoom
// location 2 : screen_size
uniform vec2 map_size;
uniform float time;

vec4 gamma_correct(in vec4 colour);

vec2 get_corrected_coords(vec2 coords) {
	coords.y -= (1.f - 1.f / 1.3f) * 3.f / 5.f;
	coords.y *= 1.3f;
	return coords;
}

// The "foldable map" overlay effect
// Rewritten to be more time efficient
vec4 get_overlay_color(vec4 color) {
	vec2 a1 = vec2(11.f); //size of overlay
	vec2 a2 = vec2(1.f, map_size.y / map_size.x);
	vec2 a3 = a1 * a2;
	//
	vec4 overcolor = texture(overlay, tex_coord * a3);
	vec4 t = overcolor * color;
	vec4 u = 0.5 - (1.f - overcolor) * (1.f - color);
	vec4 v = 2.f * mix(t, u, overcolor.r * OVERLAY_MIX);
	return vec4(v.r, v.g, v.b, 1.f);
}

vec3 Hue(float H) {
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	vec3 c = vec3(R, G, B);
	return c;
	//return c * vec3(0.299f, 0.587f, 0.114f);
}
vec3 HSVtoRGB(vec3 HSV) {
	return ((Hue(HSV.x) - 1) * HSV.y + 1) * HSV.z;
}
vec3 RGBtoHSV(vec3 RGB) {
	vec3 HSV = vec3(0);
	HSV.z = max(RGB.r, max(RGB.g, RGB.b));
	float M = min(RGB.r, min(RGB.g, RGB.b));
	float C = HSV.z - M;
	if (C != 0) {
		HSV.y = C / HSV.z;
		vec3 Delta = (HSV.z - RGB) / C;
		Delta.rgb -= Delta.brg;
		Delta.rg += vec2(2, 4);
		if (RGB.r >= HSV.z)
			HSV.x = Delta.b;
		else if (RGB.g >= HSV.z)
			HSV.x = Delta.r;
		else
			HSV.x = Delta.g;
		HSV.x = mod(HSV.x / 6, 1.f);
	}
	return HSV;
}

// The terrain map
// No province color is used here
void main() {
	// sheet is composed of 64 files, in 4 cubes of 4 rows of 4 columns
	// so each column has 8 tiles, and each row has 8 tiles too
	float xx = 1.f / map_size.x;
	float yy = 1.f / map_size.y;
	vec2 t1 = floor(tex_coord * map_size + vec2(0.5f));
	float index = floor(texture(terrain_texture_sampler, t1 / map_size).r * 256.f);
	// Float 0 to 1, is_water_01 is only either 0 or 1
	float is_water = step(64.f, index);
	float is_water_01 = min(1.f, floor(0.5f + is_water));
	vec4 out_color;
	if(is_water_01 == 1.f) {
		if(HAVE_WATER_COLORMAP) {
			vec4 color = texture(colormap_water, get_corrected_coords(tex_coord));
			out_color = get_overlay_color(color);
		} else {
			vec4 color = vec4(0.21f, 0.38f, 0.45f, 1.f);
			out_color = get_overlay_color(color);
		}
	} else {
		vec2 prov_id = texture(provinces_texture_sampler, tex_coord).xy;
		// The primary and secondary map mode province colors
		vec4 prov_color = texture(province_color, vec3(prov_id, 0.f));
		vec4 stripe_color = texture(province_color, vec3(prov_id, 1.f));
		vec2 stripe_coord = tex_coord * vec2(512.f) * vec2(1.f, map_size.y / map_size.x);
		// Mix together the primary and secondary colors with the stripes
		float stripe_factor = texture(stripes_texture, stripe_coord).a;
		float prov_highlight = texture(province_highlight, prov_id).r * (abs(cos(time * 3.f)) + 1.f);
		vec4 political = mix(prov_color, stripe_color, stripe_factor) + vec4(prov_highlight);
		political.rgb -= POLITICAL_LIGHTNESS;
		vec4 background = texture(colormap_political, get_corrected_coords(tex_coord));
		// Output color
		out_color = get_overlay_color(political);
		out_color = mix(background, out_color, POLITICAL_TERRAIN_MIX);
		out_color *= COLOR_LIGHTNESS;
	}
	frag_color = out_color;
	frag_color.a = 1.f;
	frag_color = gamma_correct(frag_color);
}
