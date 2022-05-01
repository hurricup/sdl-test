#version 410 core

layout(location = 0) in vec4 vertex_color;
layout(location = 1) in vec2 vertex_tex_coord;

uniform sampler2D our_texture;

out vec4 color;

void main(){
    color = texture(our_texture, vertex_tex_coord) * vertex_color;
}