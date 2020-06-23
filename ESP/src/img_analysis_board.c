
#include "img_analysis_board.h"
#include "color.h"
#include "vec2d.h"
#include <string.h>
#include <math.h>

bool check_possible_marker(const camera_fb_t* const img, cmarker_t* const marker){
    int16_t x_min = marker->x;
    int16_t x = marker->x;
    int16_t x_max = marker->x;
    int16_t y_min = marker->y;
    int16_t y = marker->y;
    int16_t y_max = marker->y;
    
    for(uint8_t i = 0; i < 2; i++){
        for (; x_min >= 0; x_min--){
            uint8_t red = get_red( img, x_min, y) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t green = get_green( img, x_min, y) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t blue = get_blue( img, x_min, y) & (~0u << (8 - MARKER_COLOR_RES));
            if((red != 0) ||
                (!marker->origin && (green < 0x40 || blue >= 0x40)) ||
                (marker->origin && (green >= 0x40 || blue < 0x40)))
                break;
        }
        for (; x_max < img->width; x_max++){
            uint8_t red = get_red( img, x_max, y) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t green = get_green( img, x_max, y) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t blue = get_blue( img, x_max, y) & (~0u << (8 - MARKER_COLOR_RES));
            if((red != 0) ||
                (!marker->origin && (green < 0x40 || blue >= 0x40)) ||
                (marker->origin && (green >= 0x40 || blue < 0x40)))
                break;
        }

        x = (x_min + x_max) / 2;
        if((x_max - x_min) > img->height / 10){
            //printf("to larger for marker\n");
            return false;
        }

        for (; y_min >= 0; y_min--){
            uint8_t red = get_red( img, x, y_min) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t green = get_green( img, x, y_min) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t blue = get_blue( img, x, y_min) & (~0u << (8 - MARKER_COLOR_RES));
            if((red != 0) ||
                (!marker->origin && (green < 0x40 || blue >= 0x40)) ||
                (marker->origin && (green >= 0x40 || blue < 0x40)))
                break;
        }
        for (; y_max < img->height; y_max++){
            uint8_t red = get_red( img, x, y_max) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t green = get_green( img, x, y_max) & (~0u << (8 - MARKER_COLOR_RES));
            uint8_t blue = get_blue( img, x, y_max) & (~0u << (8 - MARKER_COLOR_RES));
            if((red != 0) ||
                (!marker->origin && (green < 0x40 || blue >= 0x40)) ||
                (marker->origin && (green >= 0x40 || blue < 0x40)))
                break;
        }

        y = (y_min + y_max) / 2;
        if((y_max - y_min) > img->height / 10){
            //printf("to larger for marker\n");
            return false;
        }
    }

    marker->x = (x_min + x_max) / 2;
    marker->y = (y_min + y_max) / 2;

    uint16_t x_size = x_max - x_min;
    uint16_t y_size = y_max - y_min;
    printf("(%d-%d|%d-%d) (%d|%d)\n", x_min, x_max, y_min, y_max, x_size, y_size);
    return (x_size > img->height / 32 && y_size > img->height / 32);
}

bool find_marker(const camera_fb_t* const img, cmarker_t* marker, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max){
    for (size_t y = y_min; y < y_max; y += 8){
        for (size_t x = x_min; x < x_max; x += 8){
            const uint16_t red = QUAD_COLOR_SUM(get_red, img, x, y, 3, MARKER_COLOR_RES);
            const uint16_t green = QUAD_COLOR_SUM(get_green, img, x, y, 3, MARKER_COLOR_RES);
            const uint16_t blue = QUAD_COLOR_SUM(get_blue, img, x, y, 3, MARKER_COLOR_RES);
            if(red < 0x40){
                if(green >= 0x40 * 4 && blue < 0x40 * 4){
                    //printf("g(%d|%d): %d %d %d\n", x, y, red, green, blue);
                    marker->origin = false;
                    marker->x = x;
                    marker->y = y;
                    if(check_possible_marker(img, marker)){
                        return true;
                    }
                }else if(green < 0x40 * 4 && blue >= 0x40 * 4){
                    //printf("b(%d|%d): %d %d %d\n", x, y, red, green, blue);
                    marker->origin = true;
                    marker->x = x;
                    marker->y = y;
                    if(check_possible_marker(img, marker)){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool find_markers(const camera_fb_t* const img, cmarker_t markers[4]){
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

void calculate_fields(const camera_fb_t* const img, vec2di_t fields[8][8], const cmarker_t markers[4]){
    uint8_t origin_marker = 0;
    for(uint8_t i = 0; i < 4; i++){
        if(markers[i].origin){
            origin_marker = i;
            break;
        }
    }

    printf("calculating field positions...\n");

    uint8_t x_marker = (origin_marker + 3) & 0b11;
    uint8_t corner_marker = (origin_marker + 2) & 0b11;
    uint8_t y_marker = (origin_marker + 1) & 0b11;

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
    printf("x-achsis: (%f|%f)\n", x_achsis.x, x_achsis.y);
    printf("y-achsis: (%f|%f)\n", y_achsis.x, y_achsis.y);

    for (size_t y = 0; y < 8; y++){
        for (size_t x = 0; x < 8; x++){
            vec2df_t offset_x = div_vec2df(mul_vec2df(x_achsis, x + 1), 9);
            vec2df_t offset_y = div_vec2df(mul_vec2df(y_achsis, y + 1), 9);
            vec2df_t pos = add_vec2df(add_vec2df(origin, offset_x), offset_y);
            fields[x][y] = (vec2di_t){
                    .x = round(pos.x),
                    .y = round(pos.y)
                };
        }
    }

}

cmarker_check_result_t check_markers(const camera_fb_t* const img, cmarker_t markers[4]){
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
