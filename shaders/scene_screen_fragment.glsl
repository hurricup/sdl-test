#version 430 core
out vec4 color;

in vec2 texture_position;

uniform sampler2D screen_texture;

void main()
{
    color = texture(screen_texture, texture_position);
}