typedef struct point3 {
    float x;
    float y;
    float z;
} point3_t;

#define POINT3_SIZE sizeof(point3_t)

static inline void
set_point3(point3_t *point, float x, float y, float z) {
    point->x = x;
    point->y = y;
    point->z = z;
}

#define set_vec3 set_point3

typedef point3_t vec3_t;
#define VEC3_SIZE POINT3_SIZE

typedef struct line {
    point3_t a;
    point3_t b;
} line_t;

#define LINE_SIZE sizeof(line_t)

typedef struct triangle {
    point3_t a;
    point3_t b;
    point3_t c;
} triangle_t;

#define TRIANGLE_SIZE sizeof(triangle_t)

typedef struct square {
    point3_t a;
    point3_t b;
    point3_t c;
    point3_t d;
} square_t;

#define SQUARE_SIZE sizeof(square_t)

static inline void
set_square(square_t *square,
           float x1, float y1, float z1,
           float x2, float y2, float z2,
           float x3, float y3, float z3,
           float x4, float y4, float z4) {
    set_point3(&(square->a), x1, y1, z1);
    set_point3(&(square->b), x2, y2, z2);
    set_point3(&(square->c), x3, y3, z3);
    set_point3(&(square->d), x4, y4, z4);
}

typedef struct cube {
    square_t side_a;
    square_t side_b;
} cube_t;

#define CUBE_SIZE sizeof(cube_t)
#define CUBE_SIDE_B_OFFSET SQUARE_SIZE
