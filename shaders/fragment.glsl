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

struct SpotLight {
    Light light;
    vec3 front;
    float angle_cos;
};

layout(location = 0) in vec2 tex_coord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 frag_pos;
layout(location = 3) in vec3 ambient_color;

uniform vec3 camera_pos;
uniform mat3 normals_model;
uniform Material material;
uniform Light light;
uniform SpotLight spot_light;
uniform float oscillation;

layout(binding = 0) uniform sampler2D texture1;
layout(binding = 1) uniform sampler2D texture2;
layout(binding = 2) uniform sampler2D diffuse_map;
layout(binding = 3) uniform sampler2D specular_map;

out vec4 color;

void main(){
    float attenuation_const_quadratic = 1.0 / 2000.0;

    vec3 light_frag_vector = frag_pos - light.position;
    float light_frag_distance = length(light_frag_vector);
    float light_attenuation = 1 / (light_frag_distance * light_frag_distance * attenuation_const_quadratic + 1.0);

    vec3 camera_frag_vector = frag_pos - camera_pos;
    vec3 camera_frag_direction = normalize(camera_frag_vector);
    float camera_frag_distance = length(camera_frag_vector);
    float camera_attenuation = 1 / (camera_frag_distance * camera_frag_distance * attenuation_const_quadratic + 1.0);

    // ambient_color
    vec3 textured_ambient_color = light_attenuation * ambient_color * vec3(texture(texture2, tex_coord));

    // diffuse color
    vec3 frag_normal  = normalize(normals_model * normal);
    vec3 light_frag_direction = normalize(light_frag_vector);
    float light_diffuse = max(dot(frag_normal, -light_frag_direction), 0.0);
    vec3 light_diffuse_color = light.diffuse * (light_diffuse * material.diffuse * vec3(texture(texture2, tex_coord))) * light_attenuation;

    // specular color
    vec3 light_reflect_direction = reflect(light_frag_direction, frag_normal);
    float light_specular = pow(max(dot(-camera_frag_direction, light_reflect_direction), 0.0), material.shininess);
    vec3 light_specular_color = light.specular * (light_specular * material.specular * vec3(texture(specular_map, tex_coord))) * light_attenuation;

    vec3 frag_color3 = textured_ambient_color + light_diffuse_color + light_specular_color;

    // spotlight
    if (spot_light.angle_cos > 0.0f){
        vec3 spot_front_direction = normalize(spot_light.front);
        vec3 spot_frag_light_vector = frag_pos - spot_light.light.position;
        vec3 spot_frag_direction = normalize(spot_frag_light_vector);
        float theta_cos = dot(spot_frag_direction, spot_front_direction);
        if (theta_cos > spot_light.angle_cos){
            // spot attenuation
            float spot_distance = length(spot_frag_light_vector);
            float spot_light_attenuation = 1 / (spot_distance * spot_distance * attenuation_const_quadratic + 1.0);

            // spot diffuse
            float spot_diffuse = max(dot(frag_normal, -spot_frag_direction), 0.0);
            vec3 spot_diffuse_color = spot_light.light.diffuse * (spot_diffuse * material.diffuse * vec3(texture(texture2, tex_coord))) * spot_light_attenuation;

            // spot specular
            vec3 spot_reflect_direction = reflect(spot_frag_direction, frag_normal);
            float spot_specular = pow(max(dot(-camera_frag_direction, spot_reflect_direction), 0.0), material.shininess);
            vec3 spot_specular_color = spot_light.light.specular * (spot_specular * material.specular * vec3(texture(specular_map, tex_coord))) * spot_light_attenuation;

            frag_color3 += spot_diffuse_color + spot_specular_color;
        }
    }

    vec4 frag_color = vec4(frag_color3, 1.0);
    color = mix(texture(texture1, tex_coord), texture(texture2, tex_coord), 1) * frag_color * camera_attenuation;
}