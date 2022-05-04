#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;

uniform vec3 light_color;
uniform mat4 model_m;
uniform mat4 project_view_m;

layout(location = 0) out vec4 vertex_light_color;
layout(location = 1) out vec2 vertext_tex_coord;
layout(location = 2) out float tex_transparency;

void main(){
    gl_Position = project_view_m * model_m * vec4(position, 1.0);

    vertext_tex_coord = tex_coord;
    tex_transparency = light_color.x;

    float ambient_strength = 0.1;
    vec3 ambient_color = ambient_strength * light_color;
    vertex_light_color = vec4(ambient_color, 1.0);
}