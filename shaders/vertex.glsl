#version 430 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 project_view;
uniform vec3 light_color;
uniform Material material;

layout(location = 0) out vec2 frag_tex_coord;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_pos;
layout(location = 3) out vec3 ambient_color;

void main(){
    gl_Position = project_view * model * vec4(position, 1.0);
    frag_tex_coord = tex_coord;
    frag_normal = normal;
    frag_pos = vec3(model * vec4(position, 1.0));
    ambient_color = light_color * material.ambient;
}