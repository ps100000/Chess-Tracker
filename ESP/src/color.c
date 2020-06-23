#include "color.h"
#include <stdlib.h>
#include <math.h>

uint64_t color_distance(rgb_color_t a, rgb_color_t b){
    int16_t max = abs(a.red - b.red);
    int16_t distance = abs(a.green - b.green);
    if(max < distance){
        max = distance;
    }
    distance = abs(a.blue - b.blue);
    if(max < distance){
        max = distance;
    }
    return max;
}

rgb_vec3d_t add_rgb(rgb_vec3d_t a, rgb_vec3d_t b){
    return (rgb_vec3d_t){
        .red = a.red + b.red,
        .green = a.green + b.green,
        .blue = a.blue + b.blue,
    };
};

rgb_vec3d_t div_rgb(rgb_vec3d_t a, int32_t b){
    return (rgb_vec3d_t){
        .red = a.red / b,
        .green = a.green / b,
        .blue = a.blue / b,
    };
};

rgb_vec3d_t rgb_vec3d(rgb_color_t col){
    return (rgb_vec3d_t){
        .red = col.red,
        .green = col.green,
        .blue = col.blue,
    };
}

rgb_color_t get_rgb(const camera_fb_t* const img, const uint16_t x, const uint16_t y){
    const uint32_t index = (img->width * y + x) * 3;
    return *((rgb_color_t*)(img->buf + index));
}

uint8_t get_red(const camera_fb_t* const img, const uint16_t x, const uint16_t y){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 5) & 0b111;
    const uint32_t index = (img->width * y + x) * 3;
    return img->buf[index];
}

uint8_t get_green(const camera_fb_t* const img, const uint16_t x, const uint16_t y){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 3) & 0b11;
    const uint32_t index = (img->width * y + x) * 3 + 1;
    return img->buf[index];
}

uint8_t get_blue(const camera_fb_t* const img, const uint16_t x, const uint16_t y){
    //const uint32_t index = img->width * y + x;
    //return img->buf[index] & 0b111;
    const uint32_t index = (img->width * y + x) * 3 + 2;
    return img->buf[index];
}

void set_red(const camera_fb_t* const img, const uint16_t x, const uint16_t y, const uint8_t val){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 5) & 0b111;
    const uint32_t index = (img->width * y + x) * 3 + 0;
    img->buf[index] = val;
}

void set_green(const camera_fb_t* const img, const uint16_t x, const uint16_t y, const uint8_t val){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 3) & 0b11;
    const uint32_t index = (img->width * y + x) * 3 + 1;
    img->buf[index] = val;
}

void set_blue(const camera_fb_t* const img, const uint16_t x, const uint16_t y, const uint8_t val){
    //const uint32_t index = img->width * y + x;
    //return img->buf[index] & 0b111;
    const uint32_t index = (img->width * y + x) * 3 + 2;
    img->buf[index] = val;
}
