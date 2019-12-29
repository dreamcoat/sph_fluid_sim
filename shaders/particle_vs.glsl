#version 440 core

in vec2 position;

uniform vec2 boundary_size;

void main(void)
{
	float norm_x = (float(position.x) / (boundary_size.x / 2.0)) - 1.0;
	float norm_y = (float(position.y) / (boundary_size.y / 2.0)) - 1.0;

	gl_Position = vec4(norm_x, norm_y, 1.0, 1.0);
	gl_PointSize = 10;
}
