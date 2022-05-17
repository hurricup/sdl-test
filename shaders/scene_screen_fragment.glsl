#version 430 core
#define EFFECT_NONE 0
#define EFFECT_INVERT 1
#define EFFECT_GRAYSCALE 2
#define EFFECT_SHARP 3
#define EFFECT_BLUR 4
#define EFFECT_EDGE 5

out vec4 color;

in vec2 texture_position;

uniform sampler2D screen_texture;
uniform int effect_type;
uniform float step_x;
uniform float step_y;

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
}