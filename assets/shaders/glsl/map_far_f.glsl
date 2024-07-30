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

// location 0 : offset
// location 1 : zoom
// location 2 : screen_size
uniform vec2 map_size;
uniform float time;

vec4 gamma_correct(in vec4 colour);

// sheet is composed of 64 files, in 4 cubes of 4 rows of 4 columns
// so each column has 8 tiles, and each row has 8 tiles too
float xx = 1 / map_size.x;
float yy = 1 / map_size.y;
vec2 pix = vec2(xx, yy);

vec2 get_corrected_coords(vec2 coords) {
	coords.y -= (1 - 1 / 1.3) * 3 / 5;
	coords.y *= 1.3;
	return coords;
}

vec4 get_water_political() {
	vec3 water_background = vec3(0.21, 0.38, 0.45);
	vec3 color = water_background.rgb;

	// The "foldable map" overlay effect
	vec4 OverlayColor = texture(overlay, tex_coord * vec2(11.f, 11.f * map_size.y / map_size.x));
	vec4 OutColor;
	OutColor.r = OverlayColor.r < .5 ? (2. * OverlayColor.r * color.r) : (1. - 2. * (1. - OverlayColor.r) * (1. - color.r));
	OutColor.g = OverlayColor.r < .5 ? (2. * OverlayColor.g * color.g) : (1. - 2. * (1. - OverlayColor.g) * (1. - color.g));
	OutColor.b = OverlayColor.b < .5 ? (2. * OverlayColor.b * color.b) : (1. - 2. * (1. - OverlayColor.b) * (1. - color.b));
	OutColor.a = OverlayColor.a;

	return OutColor;
}

// The terrain color from the current texture coordinate offset with one pixel in the "corner" direction
vec4 get_terrain(vec2 corner, vec2 offset) {
	float index = texture(terrain_texture_sampler, floor(tex_coord * map_size + vec2(0.5, 0.5)) / map_size + 0.5 * pix * corner).r;
	index = floor(index * 256);
	float is_water = step(64, index);
	vec4 colour = texture(terrainsheet_texture_sampler, vec3(offset, index));
	return mix(colour, vec4(0.), is_water);
}

vec4 get_land_political_far() {
	vec4 terrain = get_terrain(vec2(0, 0), vec2(0));
	float is_land = terrain.a;
	vec2 prov_id = texture(provinces_texture_sampler, tex_coord).xy;

	// The primary and secondary map mode province colors
	vec4 prov_color = texture(province_color, vec3(prov_id, 0.));
	vec4 stripe_color = texture(province_color, vec3(prov_id, 1.));

	vec2 stripe_coord = tex_coord * vec2(512., 512. * map_size.y / map_size.x);

	// Mix together the primary and secondary colors with the stripes
	float stripeFactor = texture(stripes_texture, stripe_coord).a;
	float prov_highlight = texture(province_highlight, prov_id).r * (abs(cos(time * 3.f)) + 1.f);
	vec3 political = clamp(mix(prov_color, stripe_color, stripeFactor) + vec4(prov_highlight), 0.0, 1.0).rgb;
	political = political - 0.7;

	// The "foldable map" overlay effect
	vec4 OverlayColor = texture(overlay, tex_coord * vec2(11., 11.*map_size.y/map_size.x));
	vec4 OutColor;
	OutColor.r = OverlayColor.r < .5 ? (2. * OverlayColor.r * political.r) : (1. - 2. * (1. - OverlayColor.r) * (1. - political.r));
	OutColor.g = OverlayColor.r < .5 ? (2. * OverlayColor.g * political.g) : (1. - 2. * (1. - OverlayColor.g) * (1. - political.g));
	OutColor.b = OverlayColor.b < .5 ? (2. * OverlayColor.b * political.b) : (1. - 2. * (1. - OverlayColor.b) * (1. - political.b));
	OutColor.a = OverlayColor.a;

	vec3 background = texture(colormap_political, get_corrected_coords(tex_coord)).rgb;
	OutColor.rgb = mix(background, OutColor.rgb, 0.3);

	OutColor.rgb *= 1.5;
	OutColor.a = is_land;
	return OutColor;
}

// The terrain map
// No province color is used here
void main() {
	vec4 terrain = get_land_political_far();
	vec4 water = get_water_political();
	frag_color.rgb = mix(water.rgb, terrain.rgb, min(1.f, floor(0.5f + terrain.a)));
	frag_color.a = 1.f;
	frag_color = gamma_correct(frag_color);
}
