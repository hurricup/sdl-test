#version 430 core

in vec3 frag_position;

uniform samplerCube skybox;

out vec4 color;

void main(){
    color = texture(skybox, frag_position);
}