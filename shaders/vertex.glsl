#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 project_view;

layout(location = 0) out vec2 frag_tex_coord;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_pos;

void main(){
    gl_Position = project_view * model * vec4(position, 1.0);
    frag_tex_coord = tex_coord;
    frag_normal = normal;
    frag_pos = vec3(model * vec4(position, 1.0));
}