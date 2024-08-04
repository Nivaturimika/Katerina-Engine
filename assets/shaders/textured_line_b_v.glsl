// Goes from 0 to 1
layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec2 prev_point;
layout (location = 2) in vec2 next_point;
layout (location = 3) in float texture_coord;
layout (location = 4) in float distance;

out float tex_coord;
out float o_dist;

uniform vec2 offset;
uniform float aspect_ratio;
uniform float zoom;
uniform float width;

vec4 calc_gl_position(in vec3 v);

void main() {
	vec4 central_pos = calc_gl_position(vec3(vertex_position.x, 0.f, vertex_position.y));
	central_pos /= central_pos.w;

	vec4 apt4 = calc_gl_position(vec3(prev_point.x, 0.f, prev_point.y));
	vec4 cpt4 = calc_gl_position(vec3(next_point.x, 0.f, next_point.y));

	vec2 bpt = central_pos.xy;
	vec2 apt = apt4.xy / apt4.w;
	vec2 cpt = cpt4.xy / cpt4.w;

	// we want to thicken the line in "perceived" coordinates, so
	// transform to perceived coordinates + depth
	bpt.x *= aspect_ratio;
	apt.x *= aspect_ratio;
	cpt.x *= aspect_ratio;

	// calculate normals in perceived coordinates + depth
	vec2 adir = normalize(bpt - apt);
	vec2 bdir = normalize(cpt - bpt);

	vec2 anorm = vec2(-adir.y, adir.x);
	vec2 bnorm = vec2(-bdir.y, bdir.x);
	vec2 corner_normal = normalize(anorm + bnorm);

	vec2 corner_shift = corner_normal * zoom * width / (1.0f + max(-0.5f, dot(anorm, bnorm)));

	// transform result back to screen + depth coordinates
	corner_shift.x /= aspect_ratio;

	gl_Position = central_pos + vec4(corner_shift.x, corner_shift.y, 0.0f, 0.0f);
	//This is quirky hack... and it works!
	if(apt.x < -1.f || apt.x > 1.f
	|| bpt.x < -1.f || bpt.x > 1.f
	|| cpt.x < -1.f || cpt.x > 1.f
	|| apt.y < -1.f || apt.y > 1.f
	|| bpt.y < -1.f || bpt.y > 1.f
	|| cpt.y < -1.f || cpt.y > 1.f) {
		gl_Position = central_pos;
	}

	// pass data to frag shader
	tex_coord = texture_coord;
	o_dist = distance / (2.0f * width);
}
