#version 430 core
#define TEX_TYPE_DIFFUSE 1
#define TEX_TYPE_SPECULAR 2
#define TEX_TYPE_AMBIENT 3
#define TEX_TYPE_EMISSIVE 4
#define TEX_TYPE_HEIGHT 5
#define TEX_TYPE_NORMALS 6
#define TEX_TYPE_SHININESS 7
#define TEX_TYPE_OPACITY 8
#define TEX_TYPE_DISPLACEMENT 9
#define TEX_TYPE_LIGHTMAP 10
#define TEX_TYPE_REFLECTION 11
#define MAX_TEX_TYPE TEX_TYPE_REFLECTION

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
    float opacity;
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

struct DynamicData{
    vec4 ambient;
    bool ambient_on;

    vec4 diffuse;
    bool diffuse_on;

    vec4 specular;
    bool specular_on;

    vec4 reflection;
    bool reflection_on;

    vec3 camera_frag_direction;
    float camera_attenuation;

    vec3 normal;
};

layout(location = 0) in vec2 tex_coord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 position;

uniform OmniLight omni_lights[10];
uniform int omni_lights_number;
uniform DirectLight direct_lights[10];
uniform int direct_lights_number;
uniform SpotLight spot_lights[10];
uniform int spot_lights_number;

uniform vec3 camera_position;
uniform mat3 normals_model;
uniform Material material;

uniform int textures_number[MAX_TEX_TYPE+1];
uniform sampler2D texture_diffuse0;// type 1
uniform sampler2D texture_specular0;// type 2
uniform sampler2D texture_ambient0;// type 3
uniform sampler2D texture_emissive0;// type 4
uniform sampler2D texture_height0;// type 5
uniform sampler2D texture_normals0;// type 6
uniform sampler2D texture_shininess0;// type 7
uniform sampler2D texture_opacity0;// type 8
uniform sampler2D texture_displacement0;// type 9
uniform sampler2D texture_ligthmap0;// type 10
uniform sampler2D texture_reflection0;// type 11

uniform samplerCube skybox;

out vec4 color;

float attenuation_const_quadratic = 1.0 / 8000.0;

DynamicData computeDynamicData(){
    DynamicData dd;

    dd.diffuse_on = textures_number[TEX_TYPE_DIFFUSE] > 0;
    if (dd.diffuse_on){
        dd.diffuse = texture(texture_diffuse0, tex_coord);
    }

    dd.ambient_on = textures_number[TEX_TYPE_AMBIENT] > 0;
    if (dd.ambient_on){
        dd.ambient = texture(texture_ambient0, tex_coord);
    }
    else if (dd.diffuse_on){
        dd.ambient = dd.diffuse;
    }

    dd.specular_on = textures_number[TEX_TYPE_SPECULAR] > 0;
    if (dd.specular_on){
        dd.specular = texture(texture_specular0, tex_coord);
    }

    dd.reflection_on = textures_number[TEX_TYPE_REFLECTION] > 0;
    if (dd.reflection_on){
        dd.reflection = texture(texture_reflection0, tex_coord);
    }

    // camera distance attuniation
    vec3 camera_frag_vector = position - camera_position;
    dd.camera_frag_direction = normalize(camera_frag_vector);
    float camera_frag_distance = length(camera_frag_vector);
    dd.camera_attenuation = 1 / (camera_frag_distance * camera_frag_distance * attenuation_const_quadratic + 1.0);

    // fragment normal
    if (textures_number[TEX_TYPE_NORMALS] > 0){
        dd.normal = normalize(normals_model * (vec3(texture(texture_normals0, tex_coord)) * 2.0 - 1.0));
    }
    else {
        dd.normal = normalize(normals_model * normal);
    }
    return dd;
}

vec4 computeOmniLight(OmniLight omni_light, DynamicData dd){
    vec4 frag_color = vec4(0.0f);

    // omni attuniation
    vec3 light_frag_vector = position - omni_light.position;
    float light_frag_distance = length(light_frag_vector);
    float light_attenuation = 1 / (light_frag_distance * light_frag_distance * attenuation_const_quadratic + 1.0);

    // ambient_color
    vec4 light_ambient_color = omni_light.light_prop.ambient * material.ambient * light_attenuation;
    light_ambient_color.w = material.opacity;
    if (dd.ambient_on){
        light_ambient_color *= dd.ambient;
    }
    frag_color += light_ambient_color;

    // diffuse color
    vec3 light_frag_direction = normalize(light_frag_vector);
    float light_diffuse = max(dot(dd.normal, -light_frag_direction), 0.0);
    vec4 light_diffuse_color = omni_light.light_prop.diffuse * material.diffuse * light_diffuse * light_attenuation;
    light_diffuse_color.w = material.opacity;
    if (dd.diffuse_on){
        light_diffuse_color *= dd.diffuse;
    }

    frag_color += light_diffuse_color;

    // specular color
    if (material.shininess > 0){
        vec3 light_reflect_direction = reflect(light_frag_direction, dd.normal);
        float light_specular = pow(max(dot(-dd.camera_frag_direction, light_reflect_direction), 0.0), material.shininess);
        vec4 light_specular_color = omni_light.light_prop.specular * material.specular * light_specular * light_attenuation;
        if (dd.specular_on){
            light_specular_color *= dd.specular;
        }
        frag_color += light_specular_color;
    }
    return frag_color;
}

vec4 computeDirectLight(DirectLight direct_light, DynamicData dd){
    vec4 frag_color = vec4(0.0f);

    // direct ambient
    vec4 direct_ambient_color = direct_light.light_prop.ambient * material.ambient;
    direct_ambient_color.w = material.opacity;
    if (dd.ambient_on){
        direct_ambient_color *= dd.ambient;
    }
    frag_color += direct_ambient_color;

    // direct diffuse
    vec3 direct_light_frag_direction = normalize(direct_light.front);
    float direct_light_diffuse = max(dot(dd.normal, -direct_light_frag_direction), 0.0);
    vec4 direct_light_diffuse_color = direct_light.light_prop.diffuse * material.diffuse * direct_light_diffuse;
    direct_light_diffuse_color.w = material.opacity;
    if (dd.diffuse_on){
        direct_light_diffuse_color *= dd.diffuse;
    }
    frag_color += direct_light_diffuse_color;

    // direct specular
    if (material.shininess > 0){
        vec3 direct_light_reflect_direction = reflect(direct_light_frag_direction, dd.normal);
        float direct_light_specular = pow(max(dot(-dd.camera_frag_direction, direct_light_reflect_direction), 0.0), material.shininess);
        vec4 direct_light_specular_color = direct_light.light_prop.specular  * material.specular * direct_light_specular;
        if (dd.specular_on){
            direct_light_specular_color *= dd.specular;
        }
        frag_color += direct_light_specular_color;
    }
    return frag_color;
}

vec4 computeSpotLight(SpotLight spot_light, DynamicData dd){
    vec4 frag_color = vec4(0.0f);

    vec3 spot_front_direction = normalize(spot_light.front);
    vec3 spot_frag_light_vector = position - spot_light.position;
    vec3 spot_frag_direction = normalize(spot_frag_light_vector);
    float theta_cos = dot(spot_frag_direction, spot_front_direction);

    // spot attenuation
    float spot_distance = length(spot_frag_light_vector);
    float spot_light_attenuation = 1 / (spot_distance * spot_distance * attenuation_const_quadratic + 1.0);

    // spot ambient
    vec4 spot_ambient_color = spot_light.light_prop.ambient * material.ambient * spot_light_attenuation;
    spot_ambient_color.w = material.opacity;
    if (dd.ambient_on){
        spot_ambient_color *= dd.ambient;
    }
    frag_color += spot_ambient_color;

    if (theta_cos > spot_light.smooth_angle_cos){
        // attenuation adjustment for the smooth border
        if (theta_cos < spot_light.angle_cos){
            spot_light_attenuation *= (theta_cos - spot_light.smooth_angle_cos) / (spot_light.angle_cos - spot_light.smooth_angle_cos);
        }

        // spot diffuse
        float spot_diffuse = max(dot(dd.normal, -spot_frag_direction), 0.0);
        vec4 spot_diffuse_color = spot_light.light_prop.diffuse * material.diffuse * spot_diffuse * spot_light_attenuation;
        spot_diffuse_color.w = material.opacity;
        if (dd.diffuse_on){
            spot_diffuse_color *= dd.diffuse;
        }
        frag_color += spot_diffuse_color;

        // spot specular
        if (material.shininess > 0){
            vec3 spot_reflect_direction = reflect(spot_frag_direction, dd.normal);
            float spot_specular = pow(max(dot(-dd.camera_frag_direction, spot_reflect_direction), 0.0), material.shininess);
            vec4 spot_specular_color = spot_light.light_prop.specular * material.specular * spot_specular * spot_light_attenuation;
            if (dd.specular_on){
                spot_specular_color *= dd.specular;
            }
            frag_color += spot_specular_color;
        }
    }
    return frag_color;
}

vec4 computeReflection(vec4 current_color, DynamicData dd){
    vec3 camera_reflection = normalize(reflect(dd.camera_frag_direction, dd.normal));
    vec4 reflected_color = dd.reflection * texture(skybox, camera_reflection);
    return reflected_color + current_color * vec4(vec3(1) - vec3(dd.reflection), 1.0);
}

void main(){

    DynamicData dd = computeDynamicData();

    vec4 frag_color = vec4(0);

    for (int i = 0; i < omni_lights_number; i++){
        frag_color += computeOmniLight(omni_lights[i], dd);
    }

    for (int i = 0; i < direct_lights_number; i++){
        frag_color += computeDirectLight(direct_lights[i], dd);
    }

    for (int i = 0; i < spot_lights_number; i++){
        frag_color += computeSpotLight(spot_lights[i], dd);
    }

    if (dd.reflection_on){
        frag_color = computeReflection(frag_color, dd);
    }

    color = frag_color * dd.camera_attenuation;
}