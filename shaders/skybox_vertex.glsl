#version 430 core
layout (location = 0) in vec3 position;

uniform mat4 project_view;

out vec3 frag_position;

void main()
{
    frag_position = position;
    vec4 real_position = project_view * vec4(position, 1.0);
    gl_Position = real_position.xyww;
}