#version 430 core

uniform vec3 index_color;

out vec4 color;

void main(){
    color = vec4(index_color, 1.0f);
}