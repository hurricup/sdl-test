#version 430 core

layout(location = 0) in vec4 vertex_light_color;
layout(location = 1) in vec2 vertex_tex_coord;
layout(location = 2) in float tex_transparency;

layout(binding = 0) uniform sampler2D texture1;
layout(binding = 1) uniform sampler2D texture2;

out vec4 color;

void main(){
    color = mix(texture(texture1, vertex_tex_coord), texture(texture2, vertex_tex_coord), tex_transparency) * vertex_light_color;
}