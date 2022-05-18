#version 430 core
#define MAX_TEX_TYPE 8

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

out vec4 color;

void main(){
    color = vec4(1.0f);
}