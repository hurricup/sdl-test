#version 430 core

layout(location = 0) in vec3 position;

uniform vec3 passed_color;
uniform mat4 model_m;
uniform mat4 project_view_m;

layout(location = 0) out vec4 vertex_color;

void main(){
    gl_Position = project_view_m * model_m * vec4(position, 1.0);
    vertex_color = vec4(passed_color, 1.0);
}