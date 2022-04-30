typedef struct color {
    float red;
    float green;
    float blue;
} color_t;

#define COLOR_SIZE sizeof color_t

typedef struct point3 {
    float x;
    float y;
    float z;
} point3_t;

#define POINT3_SIZE sizeof point3_t

typedef point3_t vec3_t;
#define VEC3_SIZE POINT3_SIZE

typedef struct line {
    point3_t a;
    point3_t b;
} line_t;

#define LINE_SIZE sizeof line_t

typedef struct triangle {
    point3_t a;
    point3_t b;
    point3_t c;
} triangle_t;

#define TRIANGLE_SIZE sizeof triangle_t

typedef struct square {
    point3_t a;
    point3_t b;
    point3_t c;
    point3_t d;
} square_t;

#define SQUARE_SIZE sizeof square_t

typedef struct square_colored {
    square_t square;
    color_t color;
} square_colored_t;

#define SQUARE_COLORED_SIZE sizeof square_colored_t
#define SQUARE_COLORED_OFFSET_COLOR SQUARE_SIZE

typedef struct axes {
    vec3_t ax;
    vec3_t ay;
    vec3_t az;
} axes_t;

#define AXES_SIZE sizeof axes_t

typedef struct cube {
    square_t side_a;
    square_t side_b;
} cube_t;

#define CUBE_SIZE sizeof cube_t
#define CUBE_SIDE_B_OFFSET SQUARE_SIZE
