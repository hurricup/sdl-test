#version 430 core

layout(location = 0) in vec4 vertex_color;
out vec4 color;

void main(){
    color = vertex_color;
}