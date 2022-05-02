#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;

uniform vec3 passed_color;
uniform mat4 cube_transformation;

layout(location = 0) out vec4 vertex_color;
layout(location = 1) out vec2 vertext_tex_coord;

void main(){
    gl_Position = cube_transformation * vec4(position, 1.0);
    vertex_color = vec4(passed_color, 1.0);
    vertext_tex_coord = tex_coord;
}