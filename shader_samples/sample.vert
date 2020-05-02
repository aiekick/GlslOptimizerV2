#version 450

layout(location = 0) in vec2 a_position; // Current vertex position

void main()
{
	gl_Position = vec4(a_position, 0.0, 1.0);
}
