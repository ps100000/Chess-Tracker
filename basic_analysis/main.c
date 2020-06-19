#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "lodepng.h"
#include "vec2d.h"
#include "color.h"

#define D(d) //d

#define QUAD_COLOR_SUM(getter, img, x, y, offset)  (getter(img, x, y) + \
                                                    getter(img, x + offset, y) + \
                                                    getter(img, x, y + offset) + \
                                                    getter(img, x + offset, y + offset))


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

struct color_calibration{ // 0 white field 1 black field
    rgb_color_t empty[2];
    rgb_color_t white_piece[2];
    rgb_color_t black_piece[2];
};
typedef struct color_calibration color_calibration_t;


enum field_state{
    FIELD_EMPTY,
    FIELD_WHITE_PIECE,
    FIELD_BLACK_PIECE,
    FIELD_UNKNOWN
};

typedef enum field_state field_state_t;

bool check_possible_marker(const image_t* const img, cmarker_t* const marker){
    int16_t x_min = marker->x;
    int16_t x = marker->x;
    int16_t x_max = marker->x;
    int16_t y_min = marker->y;
    int16_t y = marker->y;
    int16_t y_max = marker->y;
    uint8_t expected_blue = (marker->origin) ? 0x80 : 0x00;
    for(uint8_t i = 0; i < 2; i++){
        for (; x_min >= 0; x_min--){
            uint8_t red = get_red( img, x_min, y);
            uint8_t green = get_green( img, x_min, y);
            uint8_t blue = get_blue( img, x_min, y);
            if((red != 0) || (green != 0x80) || (blue != expected_blue))
                break;
        }
        for (; x_max < img->width; x_max++){
            uint8_t red = get_red( img, x_max, y);
            uint8_t green = get_green( img, x_max, y);
            uint8_t blue = get_blue( img, x_max, y);
            if((red != 0) || (green != 0x80) || (blue != expected_blue))
                break;
        }

        x = (x_min + x_max) / 2;
        if((x_max - x_min) > img->height / 10){
            //printf("to larger for marker\n");
            return false;
        }

        for (; y_min >= 0; y_min--){
            uint8_t red = get_red( img, x, y_min);
            uint8_t green = get_green( img, x, y_min);
            uint8_t blue = get_blue( img, x, y_min);
            if((red != 0) || (green != 0x80) || (blue != expected_blue))
                break;
        }
        for (; y_max < img->height; y_max++){
            uint8_t red = get_red( img, x, y_max);
            uint8_t green = get_green( img, x, y_max);
            uint8_t blue = get_blue( img, x, y_max);
            if((red != 0) || (green != 0x80) || (blue != expected_blue))
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

    printf("(%d-%d|%d-%d)\n", x_min, x_max, y_min, y_max);
    uint16_t x_size = x_max - x_min;
    uint16_t y_size = y_max - y_min;
    return (x_size > img->height / 16 && y_size > img->height / 16);
}

bool find_marker( image_t* const img, cmarker_t* marker, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max){
    for (size_t y = y_min; y < y_max; y += 8){
        for (size_t x = x_min; x < x_max; x += 8){
            const uint16_t red = QUAD_COLOR_SUM(get_red, img, x, y, 3);
            const uint16_t green = QUAD_COLOR_SUM(get_green, img, x, y, 3);
            const uint16_t blue = QUAD_COLOR_SUM(get_blue, img, x, y, 3);
            if(red == 0x00){
                if(green == 0x80 * 4 && blue == 0x00){
                    //printf("g(%d|%d): %d %d %d\n", x, y, red, green, blue);
                    marker->origin = false;
                    marker->x = x;
                    marker->y = y;
                    if(check_possible_marker(img, marker)){
                        D(set_red(img, marker->x, marker->y, 0xf0));
                        D(set_green(img, marker->x, marker->y, 0));
                        D(set_blue(img, marker->x, marker->y, 0));
                        return true;
                    }
                    D(set_red(img, x, y, 0));
                    D(set_green(img, x, y, 0));
                    D(set_blue(img, x, y, 0));
                    D(set_red(img, x + 1, y, 0));
                    D(set_green(img, x + 1, y, 0xf0));
                    D(set_blue(img, x + 1, y, 0));
                }else if(green == 0x80 * 4 && blue == 0x80 * 4){
                    //printf("b(%d|%d): %d %d %d\n", x, y, red, green, blue);
                    marker->origin = true;
                    marker->x = x;
                    marker->y = y;
                    if(check_possible_marker(img, marker)){
                        D(set_red(img, marker->x, marker->y, 0xf0));
                        D(set_green(img, marker->x, marker->y, 0));
                        D(set_blue(img, marker->x, marker->y, 0));
                        return true;
                    }
                    D(set_red(img, x, y, 0));
                    D(set_green(img, x, y, 0));
                    D(set_blue(img, x, y, 0));
                    D(set_red(img, x + 1, y, 0));
                    D(set_green(img, x + 1, y, 0));
                    D(set_blue(img, x + 1, y, 0xf0));
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

    uint8_t error = lodepng_encode32_file("marker.png", img->buf, img->width, img->height);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

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
            D(set_red(img, fields[x][y].x, fields[x][y].y, 0));
            D(set_green(img, fields[x][y].x, fields[x][y].y, 0xff));
            D(set_blue(img, fields[x][y].x, fields[x][y].y, 0));
        }
    }
    uint8_t error = lodepng_encode32_file("fields.png", img->buf, img->width, img->height);
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

rgb_color_t avg_color_field(const image_t* const img, const vec2di_t field){
    int32_t x_min = field.x - img->height / 40;
    if(x_min < 0){
        x_min = 0;
    }
    int32_t x_max = field.x + img->height / 40;
    if(x_min > img->width - 1){
        x_min = img->width - 1;
    }
    int32_t y_min = field.y - img->height / 40;
    if(y_min < 0){
        y_min = 0;
    }
    int32_t y_max = field.y + img->height / 40;
    if(y_min > img->height - 1){
        y_min = img->height - 1;
    }

    rgb_vec3d_t color = {0,0,0};
    for(uint32_t y = y_min; y < y_max; y++){
        for(uint32_t x = x_min; x < x_max; x++){
            color = add_rgb(color, rgb_vec3d(get_rgb(img, x, y)));
        }
    }
    const uint32_t div = (x_max - x_min) * (y_max - y_min);
    color = div_rgb(color, div);
    return (rgb_color_t){color.red, color.green, color.blue};
}

void calibrate_colors(const image_t* const img, const vec2di_t fields[8][8], color_calibration_t* const calibration){
    rgb_vec3d_t color = {0,0,0};
    for(uint8_t i = 0; i < 8; i++){
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][3 - (i & 1)])));
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][5 - (i & 1)])));
    }
    color = div_rgb(color, 16);
    calibration->empty[0] = (rgb_color_t){color.red, color.green, color.blue};

    color.red = 0;
    color.green = 0;
    color.blue = 0;
    for(uint8_t i = 0; i < 8; i++){
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][2 + (i & 1)])));
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][4 + (i & 1)])));
    }
    color = div_rgb(color, 16);
    calibration->empty[1] = (rgb_color_t){color.red, color.green, color.blue};

    color.red = 0;
    color.green = 0;
    color.blue = 0;
    for(uint8_t i = 0; i < 8; i++){
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][1 - (i & 1)])));
    }
    color = div_rgb(color, 8);
    calibration->white_piece[0] = (rgb_color_t){color.red, color.green, color.blue};
    

    color.red = 0;
    color.green = 0;
    color.blue = 0;
    for(uint8_t i = 0; i < 8; i++){
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][(i & 1)])));
    }
    color = div_rgb(color, 8);
    calibration->white_piece[1] = (rgb_color_t){color.red, color.green, color.blue};

    color.red = 0;
    color.green = 0;
    color.blue = 0;
    for(uint8_t i = 0; i < 8; i++){
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][7 - (i & 1)])));
    }
    color = div_rgb(color, 8);
    calibration->black_piece[0] = (rgb_color_t){color.red, color.green, color.blue};
    

    color.red = 0;
    color.green = 0;
    color.blue = 0;
    for(uint8_t i = 0; i < 8; i++){
        color = add_rgb(color, rgb_vec3d(avg_color_field(img, fields[i][6 + (i & 1)])));
    }
    color = div_rgb(color, 8);
    calibration->black_piece[1] = (rgb_color_t){color.red, color.green, color.blue};

    uint8_t error = lodepng_encode32_file("color.png", img->buf, img->width, img->height);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

field_state_t get_field_state(const image_t* const img, const vec2di_t field, bool black_field, const color_calibration_t* const calibration){

    rgb_color_t field_color = avg_color_field(img, field);
    printf("%d (%d|%d)\n", black_field, field.x, field.y);
    field_state_t state = FIELD_EMPTY;
    uint64_t min_distance = color_distance(field_color, calibration->empty[black_field]);
    
    uint64_t distance = color_distance(field_color, calibration->white_piece[black_field]);
    if(min_distance > distance){
        state = FIELD_WHITE_PIECE;
    }

    distance = color_distance(field_color, calibration->black_piece[black_field]);
    if(min_distance > distance){
        state = FIELD_BLACK_PIECE;
    }
    printf("min_distance %d\n", min_distance);
    //if(min_distance < 40){
        return state;
    //}
    return FIELD_UNKNOWN;
}

int main(int argc, char const *argv[]){

    static vec2di_t fields[8][8];
    static field_state_t board_state[8][8] = {
        {FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE},
        {FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE},
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE},
        {FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE}
    };
    static color_calibration_t calibration;

    unsigned error;
    unsigned char* img_buf = 0;
    unsigned w, h;
    error = lodepng_decode32_file(&img_buf, &w, &h, "capture0.png");
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
            set_red(&img, x, y, get_red(&img, x, y) & 0b10000000);
            set_green(&img, x, y, get_green(&img, x, y) & 0b10000000);
            set_blue(&img, x, y, get_blue(&img, x, y) & 0b10000000);
        }
    }
    error = lodepng_encode32_file("low_color.png", img_buf, w, h);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    /*image_t img_small = {
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
*/

    //while
    bool first = true;
    {
        cmarker_t markers[4];
        if(find_markers(&img, markers)){
            calculate_fields(&img, fields, markers);
        }
        if(first){
            calibrate_colors(&img, fields, &calibration);
            printf("white empty:(%d|%d|%d)\n", calibration.empty[0].red, calibration.empty[0].green, calibration.empty[0].blue);
            printf("black empty:(%d|%d|%d)\n", calibration.empty[1].red, calibration.empty[1].green, calibration.empty[1].blue);
            printf("white white_piece:(%d|%d|%d)\n", calibration.white_piece[0].red, calibration.white_piece[0].green, calibration.white_piece[0].blue);
            printf("black white_piece:(%d|%d|%d)\n", calibration.white_piece[1].red, calibration.white_piece[1].green, calibration.white_piece[1].blue);
            printf("white black_piece:(%d|%d|%d)\n", calibration.black_piece[0].red, calibration.black_piece[0].green, calibration.black_piece[0].blue);
            printf("black black_piece:(%d|%d|%d)\n", calibration.black_piece[1].red, calibration.black_piece[1].green, calibration.black_piece[1].blue);
        }
        for(int i = 0; i < 1; i++){
            cmarker_check_result_t check_marker_result = check_markers(&img,markers);
            if(check_marker_result == CM_CHECK_ADJUSTED){
                printf("CM_CHECK_ADJUSTED\n");
                calculate_fields(&img, fields, markers);
            }else if(check_marker_result == CM_CHECK_INVALID){
                printf("CM_CHECK_INVALID\n");
                //break;
            }else{
                printf("CM_CHECK_NO_CHANGE\n");
            }
            for (size_t y = 0; y < 8; y++){
                for (size_t x = 0; x < 8; x++){
                    field_state_t current_state = get_field_state(&img, fields[x][y], !((x & 1) ^ (y & 1)), &calibration);
                    if(board_state[y][x] != current_state){
                        if(current_state == FIELD_UNKNOWN){
                            printf("Ich bin dumm: (%d|%d)\n", x, y);
                        }
                        printf("update at (%d|%d)\n", x, y);
                        board_state[y][x] = current_state;
                    }
                }
            }
            for (size_t y = 0; y < 8; y++){
                for (size_t x = 0; x < 8; x++){
                    switch(board_state[y][x]){
                    case FIELD_EMPTY:
                        printf(" ");
                        break;
                    case FIELD_UNKNOWN:
                        printf("x");
                        break;
                    case FIELD_BLACK_PIECE:
                        printf("#");
                        break;
                    case FIELD_WHITE_PIECE:
                        printf("+");
                        break;
                    default:
                        break;
                    }
                }
                printf("\n");
            }
        }
    }

    free(img_buf);
    return 0;
}
