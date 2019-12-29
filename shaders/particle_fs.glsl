#version 440 core

out vec4 color_out;

void main(void)
{
	vec2 circ_coord = 2.0 * gl_PointCoord - 1.0;
	if (dot(circ_coord, circ_coord) > 1.0)
		discard;
	color_out = vec4(0.2, 0.6, 1.0, 1.0);
}
