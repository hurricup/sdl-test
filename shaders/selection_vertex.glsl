#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

uniform mat4 model;
uniform mat4 project_view;

void main(){
    gl_Position = project_view * model * vec4(position, 1.0);
}