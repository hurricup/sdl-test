#version 330 core

layout(location = 0) in vec3 position;

uniform vec3 passed_color;

out vec4 vertexColor;

void main(){
    gl_Position = vec4(position, 1.0);
    vertexColor = vec4(passed_color, 1.0);
}