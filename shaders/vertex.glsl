#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;

uniform vec3 light_color;
uniform vec3 light_pos;
uniform mat4 model;
uniform mat3 normals_model;
uniform mat4 project_view;

layout(location = 0) out vec4 frag_light_color;
layout(location = 1) out vec2 frag_tex_coord;
layout(location = 2) out float tex_transparency;

void main(){
    gl_Position = project_view * model * vec4(position, 1.0);

    frag_tex_coord = tex_coord;
    tex_transparency = light_color.x;

    vec3 frag_pos = vec3(model * vec4(position, 1.0));

    vec3 norm  = normalize(normals_model * normal);
    vec3 light_direction = normalize(light_pos - frag_pos);
    float diffuse = max(dot(norm, light_direction), 0.0);

    float ambient_strength = 0.1;
    vec3 ambient_color = (ambient_strength + diffuse)* light_color;
    frag_light_color = vec4(ambient_color, 1.0);
}