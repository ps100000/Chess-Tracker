#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>
#include <stdbool.h>

struct image{
    unsigned char * const buf;
    uint16_t width;
    uint16_t height;
};
typedef struct image image_t;

struct rgb_color{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
typedef struct rgb_color rgb_color_t;

uint64_t color_distance(rgb_color_t a, rgb_color_t b);

struct rgb_vec3d{
    int32_t red;
    int32_t green;
    int32_t blue;
};
typedef struct rgb_vec3d rgb_vec3d_t;

rgb_vec3d_t add_rgb(rgb_vec3d_t a, rgb_vec3d_t b);
rgb_vec3d_t div_rgb(rgb_vec3d_t a, int32_t b);
rgb_vec3d_t rgb_vec3d(rgb_color_t col);

rgb_color_t get_rgb(const image_t* const img, const uint16_t x, const uint16_t y);

uint8_t get_red(const image_t* const img, const uint16_t x, const uint16_t y);
uint8_t get_green(const image_t* const img, const uint16_t x, const uint16_t y);
uint8_t get_blue(const image_t* const img, const uint16_t x, const uint16_t y);

void set_red(const image_t* const img, const uint16_t x, const uint16_t y, const uint8_t val);
void set_green(const image_t* const img, const uint16_t x, const uint16_t y, const uint8_t val);
void set_blue(const image_t* const img, const uint16_t x, const uint16_t y, const uint8_t val);

#endif
