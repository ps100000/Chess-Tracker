#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "lodepng.h"
#include "vec2d.h"

#define D(d) //d

#define QUAD_COLOR_SUM(getter, img, x, y, offset)  (getter(img, x, y) + \
                                                    getter(img, x + offset, y) + \
                                                    getter(img, x, y + offset) + \
                                                    getter(img, x + offset, y + offset))

struct image{
    unsigned char * const buf;
    uint16_t width;
    uint16_t height;
};
typedef struct image image_t;

enum cmarker_check_result{
    CM_CHECK_NO_CHANGE,
    CM_CHECK_ADJUSTED,
    CM_CHECK_INVALID
};

typedef enum cmarker_check_result cmarker_check_result_t;

struct cmarker{
    float x;
    float y;
    bool origin;
};
typedef struct cmarker cmarker_t;

struct rgb_color{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
typedef struct rgb_color rgb_color_t;

uint8_t get_red(const image_t* const img, const uint16_t x, const uint16_t y){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 5) & 0b111;
    const uint32_t index = (img->width * y + x) * 4;
    return img->buf[index] >> 5;
}

uint8_t get_green(const image_t* const img, const uint16_t x, const uint16_t y){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 3) & 0b11;
    const uint32_t index = (img->width * y + x) * 4 + 1;
    return img->buf[index] >> 6;
}

uint8_t get_blue(const image_t* const img, const uint16_t x, const uint16_t y){
    //const uint32_t index = img->width * y + x;
    //return img->buf[index] & 0b111;
    const uint32_t index = (img->width * y + x) * 4 + 2;
    return img->buf[index] >> 5;
}

void set_red(const image_t* const img, const uint16_t x, const uint16_t y, const uint8_t val){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 5) & 0b111;
    const uint32_t index = (img->width * y + x) * 4 + 0;
    img->buf[index] = val << 5;
}

void set_green(const image_t* const img, const uint16_t x, const uint16_t y, const uint8_t val){
    //const uint32_t index = img->width * y + x;
    //return (img->buf[index] >> 3) & 0b11;
    const uint32_t index = (img->width * y + x) * 4 + 1;
    img->buf[index] = val << 6;
}

void set_blue(const image_t* const img, const uint16_t x, const uint16_t y, const uint8_t val){
    //const uint32_t index = img->width * y + x;
    //return img->buf[index] & 0b111;
    const uint32_t index = (img->width * y + x) * 4 + 2;
    img->buf[index] = val << 5;
}

bool check_possible_marker(const image_t* const img, cmarker_t* const marker){
    uint16_t x_min = marker->x;
    uint16_t x = marker->x;
    uint16_t x_max = marker->x;
    uint16_t y_min = marker->y;
    uint16_t y = marker->y;
    uint16_t y_max = marker->y;
    for(uint8_t i = 0; i < 2; i++){
        for (; x_min >= 0; x_min--){
            uint8_t red = get_red( img, x_min, y);
            uint8_t green = get_green( img, x_min, y);
            uint8_t blue = get_blue( img, x_min, y);
            if((red > 2) ||
                (!marker->origin && (green < 2 || blue > 2)) ||
                (marker->origin && (green > 2 || blue < 4)))
                break;
        }
        for (; x_max < img->width; x_max++){
            uint8_t red = get_red( img, x_max, y);
            uint8_t green = get_green( img, x_max, y);
            uint8_t blue = get_blue( img, x_max, y);
            if((red > 2) ||
                (!marker->origin && (green < 2 || blue > 2)) ||
                (marker->origin && (green > 2 || blue < 4)))
                break;
        }

        x = (x_min + x_max) / 2;

        for (; y_min >= 0; y_min--){
            uint8_t red = get_red( img, x, y_min);
            uint8_t green = get_green( img, x, y_min);
            uint8_t blue = get_blue( img, x, y_min);
            if((red > 2) ||
                (!marker->origin && (green < 2 || blue > 2)) ||
                (marker->origin && (green > 2 || blue < 4)))
                break;
        }
        for (; y_max < img->width; y_max++){
            uint8_t red = get_red( img, x, y_max);
            uint8_t green = get_green( img, x, y_max);
            uint8_t blue = get_blue( img, x, y_max);
            if((red > 2) ||
                (!marker->origin && (green < 2 || blue > 2)) ||
                (marker->origin && (green > 2 || blue < 4)))
                break;
        }

        y = (y_min + y_max) / 2;
    }

    marker->x = (x_min + x_max) / 2;
    marker->y = (y_min + y_max) / 2;
    printf("(%d-%d|%d-%d)\n", x_min, x_max, y_min, y_max);
    return ((x_max - x_min) > img->height / 16 && (y_max - y_min) > img->height / 16);
}

bool find_marker(const image_t* const img, cmarker_t* marker, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max){
    for (size_t y = y_min; y < y_max - 3; y += 3){
        for (size_t x = x_min; x < x_max - 3; x += 3){
            const uint8_t red = QUAD_COLOR_SUM(get_red, img, x, y, 3);
            const uint8_t green = QUAD_COLOR_SUM(get_green, img, x, y, 3);
            const uint8_t blue = QUAD_COLOR_SUM(get_blue, img, x, y, 3);
            if(red <= 8){
                if(green >= 7 && blue <= 8){
                    printf("g(%d|%d): %d %d %d\n", x, y, red, green, blue);
                    marker->origin = false;
                    marker->x = x;
                    marker->y = y;
                    if(check_possible_marker(img, marker)){
                        D(set_red(img, x, y, 0));
                        D(set_green(img, x, y, 0));
                        D(set_blue(img, x, y, 0));
                        D(set_red(img, x + 1, y, 0));
                        D(set_green(img, x + 1, y, 3));
                        D(set_blue(img, x + 1, y, 0));
                        D(set_red(img, marker->x, marker->y, 7));
                        D(set_green(img, marker->x, marker->y, 0));
                        D(set_blue(img, marker->x, marker->y, 0));
                        return true;
                    }
                }else if(green <= 3 && blue >= 14){
                    printf("b(%d|%d): %d %d %d\n", x, y, red, green, blue);
                    marker->origin = true;
                    marker->x = x;
                    marker->y = y;
                    if(check_possible_marker(img, marker)){
                        D(set_red(img, x, y, 0));
                        D(set_green(img, x, y, 0));
                        D(set_blue(img, x, y, 0));
                        D(set_red(img, x + 1, y, 0));
                        D(set_green(img, x + 1, y, 0));
                        D(set_blue(img, x + 1, y, 7));
                        D(set_red(img, marker->x, marker->y, 7));
                        D(set_green(img, marker->x, marker->y, 0));
                        D(set_blue(img, marker->x, marker->y, 0));
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool find_markers(/*const*/ image_t* const img, cmarker_t markers[4]){
    printf("Searching markers...\n");
    uint8_t marker_count = 0;
    marker_count += find_marker(img, markers + 0,
        0, img->width / 2,
        0, img->height / 2);
    marker_count += find_marker(img, markers + 1,
        img->width / 2, img->width,
        0, img->height / 2);
    marker_count += find_marker(img, markers + 2,
        img->width / 2, img->width,
        img->height / 2, img->height);
    marker_count += find_marker(img, markers + 3,
        0, img->width / 2,
        img->height / 2, img->height);
    uint8_t origin_counter = 0;
    for(uint8_t i = 0; i < 4; i++){
        if(markers[i].origin){
            origin_counter++;
        }
    }
    printf("Found %d marker (%d marked as origin)\n", marker_count, origin_counter);
    return (marker_count == 4 && origin_counter == 1);
};

void calculate_fields(/*const*/ image_t* const img, vec2di_t fields[8][8], const cmarker_t markers[4]){
    uint8_t origin_marker = 0;
    for(uint8_t i = 0; i < 4; i++){
        if(markers[i].origin){
            origin_marker = i;
            break;
        }
    }

    uint8_t error = lodepng_encode32_file("marker.png", img->buf, img->width, img->height);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));


    printf("calculating field positions...\n");

    uint8_t x_marker = (origin_marker + 1) & 0b11;
    uint8_t corner_marker = (origin_marker + 2) & 0b11;
    uint8_t y_marker = (origin_marker + 3) & 0b11;

    vec2df_t origin = {
            .x = markers[origin_marker].x,
            .y = markers[origin_marker].y
        };
    vec2df_t x_achsis = {
            .x = (markers[x_marker].x - (markers[origin_marker].x) + (markers[corner_marker].x - markers[y_marker].x)) / 2,
            .y = (markers[x_marker].y - (markers[origin_marker].y) + (markers[corner_marker].y - markers[y_marker].y)) / 2,
        };
    vec2df_t y_achsis = {
            .x = (markers[y_marker].x - (markers[origin_marker].x) + (markers[corner_marker].x - markers[x_marker].x)) / 2,
            .y = (markers[y_marker].y - (markers[origin_marker].y) + (markers[corner_marker].y - markers[x_marker].y)) / 2
        };
    
    for (size_t y = 0; y < 8; y++){
        for (size_t x = 0; x < 8; x++){
            vec2df_t offset_x = div_vec2df(mul_vec2df(x_achsis, x + 1), 9);
            vec2df_t offset_y = div_vec2df(mul_vec2df(y_achsis, y + 1), 9);
            vec2df_t pos = add_vec2df(add_vec2df(origin, offset_x), offset_y);
            fields[x][y] = (vec2di_t){
                    .x = round(pos.x),
                    .y = round(pos.y)
                };
            D(set_red(img, fields[x][y].x, fields[x][y].y, 0));
            D(set_green(img, fields[x][y].x, fields[x][y].y, 3));
            D(set_blue(img, fields[x][y].x, fields[x][y].y, 0));
        }
    }
    error = lodepng_encode32_file("fields.png", img->buf, img->width, img->height);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

}

cmarker_check_result_t check_markers(const image_t* const img, cmarker_t markers[4]){
    bool adjusted = false;
    for (uint8_t i = 0; i < 4; i++){
        cmarker_t marker;
        memcpy(&marker, markers + i, sizeof(cmarker_t));
        if(!check_possible_marker(img, markers + i)){
            return CM_CHECK_INVALID;
        }
        if(abs(markers[i].x - marker.x) > 0.1 || abs(markers[i].y - marker.y) > 0.1){
            printf("%d (%f|%f)->(%f|%f)\n", i, markers[i].x, markers[i].y, marker.x, marker.y);
            adjusted = true;
        } 
    }
    return adjusted ? CM_CHECK_ADJUSTED : CM_CHECK_NO_CHANGE;
}

rgb_color_t avg_color_field(image_t* img, const vec2di_t field){
    int32_t x_min = field.x - img->height / 30;
    if(x_min < 0){
        x_min = 0;
    }
    int32_t x_max = field.x + img->height / 30;
    if(x_min > img->width - 1){
        x_min = img->width - 1;
    }
    int32_t y_min = field.y - img->height / 30;
    if(y_min < 0){
        y_min = 0;
    }
    int32_t y_max = field.y + img->height / 30;
    if(y_min > img->height - 1){
        y_min = img->height - 1;
    }

    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    for(uint32_t y = y_min; y < y_max; y++){
        for(uint32_t x = x_min; x < x_max; x++){
            red += get_red( img, x, y);
            green += get_green( img, x, y);
            blue += get_blue( img, x, y);
        }
    }
    const uint32_t div = (x_max - x_min) * (y_max - y_min);
    red /= div;
    green /= div;
    blue /= div;
    return (rgb_color_t){red, green, blue};
}

void calibrate_colors(image_t* img, const vec2di_t fields[8][8]){
    
}

int main(int argc, char const *argv[]){
    unsigned error;
    unsigned char* img_buf = 0;
    unsigned w, h;
    static uint8_t img_bufs[2][128 * 128];
    static uint8_t active_buf_index = 0;
    static vec2di_t fields[8][8];

    error = lodepng_decode32_file(&img_buf, &w, &h, "test.png");
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    image_t img = {
        .buf = img_buf,
        .width = w,
        .height = h
    };

    printf("Image size:%dx%d\n", w, h);
    printf("RGB888->RGB323\n");

    for (size_t y = 0; y < h; y++){
        for (size_t x = 0; x < w; x++){
            const uint32_t index = (img.width * y + x) * 4;
            set_red(&img, x, y, get_red(&img, x, y));
            set_green(&img, x, y, get_green(&img, x, y));
            set_blue(&img, x, y, get_blue(&img, x, y));
        }
    }
    error = lodepng_encode32_file("low_color.png", img_buf, w, h);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    image_t img_small = {
            .buf = malloc((w / 2) * (h / 2) * 4),
            .width = w / 2,
            .height = h / 2
        };
    memset(img_small.buf, 255, (w / 2) * (h / 2) * 4);

    printf("%dx%d->%dx%d\n", w, h, img_small.width, img_small.height);
    for (size_t y = 0; y < h - 1; y += 2){
        const uint16_t new_y = y / 2;
        for (size_t x = 0; x < w - 1; x += 2){
            const uint16_t new_x = x / 2;
            set_red(&img_small, new_x, new_y, QUAD_COLOR_SUM(get_red, &img, x, y, 1) / 4);
            set_green(&img_small, new_x, new_y, QUAD_COLOR_SUM(get_green, &img, x, y, 1) / 4);
            set_blue(&img_small, new_x, new_y, QUAD_COLOR_SUM(get_blue, &img, x, y, 1) / 4);
        }
    }
    
    error = lodepng_encode32_file("small.png", img_small.buf, img_small.width, img_small.height);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));


    //while
    bool first = true;
    {
        cmarker_t markers[4];
        if(find_markers(&img_small, markers)){
            calculate_fields(&img_small, fields, markers);
        }
        if(first){
            calibrate_colors(&img_small, fields);
        }
        for(int i = 0; i < 10; i++){
            cmarker_check_result_t check_marker_result = check_markers(&img_small,markers);
            if(check_marker_result == CM_CHECK_ADJUSTED){
                printf("CM_CHECK_ADJUSTED\n");
                calculate_fields(&img_small, fields, markers);
            }else if(check_marker_result == CM_CHECK_INVALID){
                printf("CM_CHECK_INVALID\n");
                //break;
            }else{
                printf("CM_CHECK_NO_CHANGE\n");
            }
        }
    }


    free(img_small.buf);
    free(img_buf);
    return 0;
}
