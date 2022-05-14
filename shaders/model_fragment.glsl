#version 430 core

struct LightProp{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 emissive;
    float shininess;
};

struct OmniLight {
    LightProp light_prop;
    vec3 position;
};

struct SpotLight {
    LightProp light_prop;
    vec3 position;
    vec3 front;
    float angle_cos;
    float smooth_angle_cos;
};

struct DirectLight {
    LightProp light_prop;
    vec3 front;
};


layout(location = 0) in vec2 tex_coord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 position;

uniform OmniLight omni_light;
uniform DirectLight direct_light;
uniform SpotLight spot_light;

uniform vec3 camera_position;
uniform mat3 normals_model;
uniform Material material;

uniform sampler2D texture_diffuse0;// type 1
uniform sampler2D texture_specular0;// type 2
uniform sampler2D texture_ambient0;// type 3
uniform sampler2D texture_emissive0;// type 4
uniform sampler2D texture_height0;// type 5
uniform sampler2D texture_normals0;// type 6
uniform sampler2D texture_shininess0;// type 7
uniform sampler2D texture_opacity0;// type 8

out vec4 color;

void main(){
    float attenuation_const_quadratic = 1.0 / 15000.0;

    vec3 light_frag_vector = position - omni_light.position;
    float light_frag_distance = length(light_frag_vector);
    float light_attenuation = 1 / (light_frag_distance * light_frag_distance * attenuation_const_quadratic + 1.0);

    vec3 camera_frag_vector = position - camera_position;
    vec3 camera_frag_direction = normalize(camera_frag_vector);
    float camera_frag_distance = length(camera_frag_vector);
    float camera_attenuation = 1 / (camera_frag_distance * camera_frag_distance * attenuation_const_quadratic + 1.0);

    // ambient_color
    vec4 textured_ambient_color = omni_light.light_prop.ambient * material.ambient * light_attenuation * texture(texture_diffuse0, tex_coord);

    // diffuse color
    vec3 frag_normal  = normalize(normals_model * normal);
    vec3 light_frag_direction = normalize(light_frag_vector);
    float light_diffuse = max(dot(frag_normal, -light_frag_direction), 0.0);
    vec4 light_diffuse_color = omni_light.light_prop.diffuse * material.diffuse * light_diffuse * texture(texture_diffuse0, tex_coord) * light_attenuation;

    vec4 frag_color = textured_ambient_color + light_diffuse_color;

    // specular color
    if (material.shininess > 0){
        vec3 light_reflect_direction = reflect(light_frag_direction, frag_normal);
        float light_specular = pow(max(dot(-camera_frag_direction, light_reflect_direction), 0.0), material.shininess);
        vec4 light_specular_color = omni_light.light_prop.specular * material.specular * light_specular * texture(texture_specular0, tex_coord) * light_attenuation;
        frag_color += light_specular_color;
    }

    // direct light
    vec4 direct_ambient_color = direct_light.light_prop.ambient * material.ambient * texture(texture_diffuse0, tex_coord);

    vec3 direct_light_frag_direction = normalize(direct_light.front);
    float direct_light_diffuse = max(dot(frag_normal, -direct_light_frag_direction), 0.0);
    vec4 direct_light_diffuse_color = direct_light.light_prop.diffuse * material.diffuse * direct_light_diffuse  * texture(texture_diffuse0, tex_coord);
    frag_color += direct_ambient_color + direct_light_diffuse_color;

    // direct specular
    if (material.shininess > 0){
        vec3 direct_light_reflect_direction = reflect(direct_light_frag_direction, frag_normal);
        float direct_light_specular = pow(max(dot(-camera_frag_direction, direct_light_reflect_direction), 0.0), material.shininess);
        vec4 direct_light_specular_color = direct_light.light_prop.specular  * material.specular * direct_light_specular * texture(texture_specular0, tex_coord);
        frag_color += direct_light_specular_color;
    }

    // spotlight
    if (spot_light.angle_cos > 0.0f){
        vec3 spot_front_direction = normalize(spot_light.front);
        vec3 spot_frag_light_vector = position - spot_light.position;
        vec3 spot_frag_direction = normalize(spot_frag_light_vector);
        float theta_cos = dot(spot_frag_direction, spot_front_direction);

        // spot attenuation
        float spot_distance = length(spot_frag_light_vector);
        float spot_light_attenuation = 1 / (spot_distance * spot_distance * attenuation_const_quadratic + 1.0);

        // spot ambient
        vec4 spot_ambient_color = spot_light.light_prop.ambient * material.ambient * texture(texture_diffuse0, tex_coord) * spot_light_attenuation;
        frag_color += spot_ambient_color;

        if (theta_cos > spot_light.smooth_angle_cos){
            // attenuation adjustment for the smooth border
            if (theta_cos < spot_light.angle_cos){
                spot_light_attenuation *= (theta_cos - spot_light.smooth_angle_cos) / (spot_light.angle_cos - spot_light.smooth_angle_cos);
            }

            // spot diffuse
            float spot_diffuse = max(dot(frag_normal, -spot_frag_direction), 0.0);
            vec4 spot_diffuse_color = spot_light.light_prop.diffuse * material.diffuse * spot_diffuse * texture(texture_diffuse0, tex_coord) * spot_light_attenuation;
            frag_color += spot_diffuse_color;

            // spot specular
            if (material.shininess > 0){
                vec3 spot_reflect_direction = reflect(spot_frag_direction, frag_normal);
                float spot_specular = pow(max(dot(-camera_frag_direction, spot_reflect_direction), 0.0), material.shininess);
                vec4 spot_specular_color = spot_light.light_prop.specular * material.specular * spot_specular * texture(texture_specular0, tex_coord) * spot_light_attenuation;
                frag_color += spot_specular_color;
            }
        }
    }


    color = frag_color * camera_attenuation;
}