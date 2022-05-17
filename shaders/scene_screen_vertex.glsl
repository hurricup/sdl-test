#version 430 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 in_texture_position;

out vec2 texture_position;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    texture_position = in_texture_position;
}