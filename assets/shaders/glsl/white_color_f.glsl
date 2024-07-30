out vec4 frag_color;
vec4 gamma_correct(in vec4 colour);

void main() {
	frag_color = gamma_correct(vec4(0.85f, 0.85f, 0.85f, 1.f));
}
