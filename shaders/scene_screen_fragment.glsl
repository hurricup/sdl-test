#version 430 core
#define PI 3.1415926538
#define EFFECT_NONE 0
#define EFFECT_INVERT 1
#define EFFECT_GRAYSCALE 2
#define EFFECT_SHARP 3
#define EFFECT_BLUR 4
#define EFFECT_EDGE 5

out vec4 color;

in vec2 texture_position;

layout(binding = 0) uniform sampler2D screen_texture;
layout(binding = 1) uniform sampler2D selection_texture;

uniform int effect_type;
uniform float step_x;
uniform float step_y;
uniform float time;

float kernel_sharp[9] = float[](
-1, -1, -1,
-1, 9, -1,
-1, -1, -1
);

float kernel_edge[9] = float[](
1, 1, 1,
1, -8, 1,
1, 1, 1
);

float kernel_blur[] = float[](
1.0 / 16, 2.0 / 16, 1.0 / 16,
2.0 / 16, 4.0 / 16, 2.0 / 16,
1.0 / 16, 2.0 / 16, 1.0 / 16
);

void kernel(inout float kernel[9]){
    color = vec4(
    vec3(texture(screen_texture, texture_position + vec2(-step_x, step_y))) * kernel[0] +
    vec3(texture(screen_texture, texture_position + vec2(0.0f, step_y))) * kernel[1] +
    vec3(texture(screen_texture, texture_position + vec2(step_x, step_y))) * kernel[2] +
    vec3(texture(screen_texture, texture_position + vec2(-step_x, 0.0f))) * kernel[3] +
    vec3(texture(screen_texture, texture_position + vec2(0.0f, 0.0f))) * kernel[4] +
    vec3(texture(screen_texture, texture_position + vec2(step_x, 0.0f))) * kernel[5] +
    vec3(texture(screen_texture, texture_position + vec2(-step_x, -step_y))) * kernel[6] +
    vec3(texture(screen_texture, texture_position + vec2(0.0f, -step_y))) * kernel[7] +
    vec3(texture(screen_texture, texture_position + vec2(step_x, -step_y))) * kernel[8],
    1.0f);
}

void draw_selection(){
    if (texture(selection_texture, texture_position).x > 0){
        return;
    }
    int border_width = 2;
    int area_to_check = border_width * 2 + 1;

    float start_x = texture_position.x - border_width * step_x;
    float start_y = texture_position.y - border_width * step_y;

    vec2 position = vec2(start_x, start_y);
    for (int i = 0; i < area_to_check; i++){
        position.y = start_y;
        for (int j = 0; j < area_to_check; j++){
            if (texture(selection_texture, position).x > 0){
                float oscillation_base = 2 * PI * (10 * (texture_position.x + texture_position.y)/2 + time / 2000);
                float green_color = sin(oscillation_base) / 2 + 0.5;
                float blue_color = sin(oscillation_base + 2 * PI / 3) / 2 + 0.5;
                float red_color = sin(oscillation_base + 4 * PI / 3) / 2 + 0.5;
                vec4 selection_color = vec4(red_color, green_color, blue_color, 1);
                color = selection_color;
                return;
            }
            position.y += step_y;
        }
        position.x += step_x;
    }
}

void main()
{
    if (EFFECT_INVERT == effect_type){
        color = vec4(vec3(1 - texture(screen_texture, texture_position)), 1.0f);
    }
    else if (EFFECT_GRAYSCALE == effect_type){
        color = texture(screen_texture, texture_position);
        float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
        color = vec4(average, average, average, 1.0);
    }
    else if (EFFECT_SHARP == effect_type){
        kernel(kernel_sharp);
    }
    else if (EFFECT_BLUR == effect_type){
        kernel(kernel_blur);
    }
    else if (EFFECT_EDGE == effect_type){
        kernel(kernel_edge);
    }
    else {
        color = texture(screen_texture, texture_position);
    }

    draw_selection();
}