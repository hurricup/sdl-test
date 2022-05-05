#version 430 core

layout(location = 0) in vec2 tex_coord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 frag_pos;

uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 camera_pos;
uniform mat3 normals_model;

layout(binding = 0) uniform sampler2D texture1;
layout(binding = 1) uniform sampler2D texture2;

out vec4 color;

void main(){
    float distance_decay_const = 2000;
    float ambient_strength = 0.1;

    vec3 light_distance_vector = light_pos - frag_pos;
    float light_distance = length(light_distance_vector);
    float light_distance_decay = distance_decay_const / (light_distance * light_distance + distance_decay_const);

    vec3 norm  = normalize(normals_model * normal);
    vec3 light_direction = normalize(light_distance_vector);
    float diffuse = max(dot(norm, light_direction), 0.0) * light_distance_decay;

    vec3 camera_distance_vector = camera_pos - frag_pos;
    float camera_distance = length(camera_distance_vector);
    float camera_distance_decay = distance_decay_const / (camera_distance * camera_distance + distance_decay_const);

    vec4 frag_color = vec4((ambient_strength + diffuse) * light_color, 1.0);
    color = mix(texture(texture1, tex_coord), texture(texture2, tex_coord), light_color.x) * frag_color * camera_distance_decay;
}