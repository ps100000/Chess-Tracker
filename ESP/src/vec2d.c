#include "vec2d.h"

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
