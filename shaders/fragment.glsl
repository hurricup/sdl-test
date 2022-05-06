#version 430 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

layout(location = 0) in vec2 tex_coord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 frag_pos;
layout(location = 3) in vec3 ambient_color;

uniform vec3 camera_pos;
uniform mat3 normals_model;
uniform Material material;
uniform Light light;
uniform float oscillation;

layout(binding = 0) uniform sampler2D texture1;
layout(binding = 1) uniform sampler2D texture2;

out vec4 color;

void main(){
    float distance_decay_const = 2000;

    vec3 light_distance_vector = light.position - frag_pos;
    float light_distance = length(light_distance_vector);
    float light_distance_decay = distance_decay_const / (light_distance * light_distance + distance_decay_const);

    vec3 camera_distance_vector = camera_pos - frag_pos;
    vec3 camera_direction = normalize(camera_distance_vector);
    float camera_distance = length(camera_distance_vector);
    float camera_distance_decay = distance_decay_const / (camera_distance * camera_distance + distance_decay_const);

    // diffuse color
    vec3 norm  = normalize(normals_model * normal);
    vec3 light_direction = normalize(light_distance_vector);
    float diffuse = max(dot(norm, light_direction), 0.0);
    vec3 diffuse_color = light.diffuse * (diffuse * material.diffuse) * light_distance_decay;

    // specular color
    vec3 reflect_direction = reflect(-light_direction, norm);
    float specular = pow(max(dot(camera_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular_color = light.specular * (specular * material.specular) * light_distance_decay;

    vec4 frag_color = vec4(ambient_color + diffuse_color + specular_color, 1.0);
    color = mix(texture(texture1, tex_coord), texture(texture2, tex_coord), oscillation) * frag_color * camera_distance_decay;
}