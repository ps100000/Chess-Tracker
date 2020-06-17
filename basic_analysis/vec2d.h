#ifndef _VEC2D_H_
#define _VEC2D_H_
#include <stdint.h>

struct vec2di{
    int32_t x;
    int32_t y;
};
typedef struct vec2di vec2di_t;

vec2di_t add_vec2di(vec2di_t a, vec2di_t b){
    return (vec2di_t){
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
}

vec2di_t mul_vec2di(vec2di_t a, float b){
    return (vec2di_t){
        .x = a.x * b,
        .y = a.y * b,
    };
}

vec2di_t div_vec2di(vec2di_t a, float b){
    return (vec2di_t){
        .x = a.x / b,
        .y = a.y / b,
    };
}

struct vec2df{
    float x;
    float y;
};
typedef struct vec2df vec2df_t;

vec2df_t add_vec2df(vec2df_t a, vec2df_t b){
    return (vec2df_t){
        .x = a.x + b.x,
        .y = a.y + b.y,
    };
}

vec2df_t mul_vec2df(vec2df_t a, float b){
    return (vec2df_t){
        .x = a.x * b,
        .y = a.y * b,
    };
}

vec2df_t div_vec2df(vec2df_t a, float b){
    return (vec2df_t){
        .x = a.x / b,
        .y = a.y / b,
    };
}

#endif
