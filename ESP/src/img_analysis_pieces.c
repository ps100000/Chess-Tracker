#include "img_analysis_pieces.h"


rgb_color_t avg_color_field(const camera_fb_t* const img, const vec2di_t field){
    int32_t x_min = field.x - img->height / 60;
    if(x_min < 0){
        x_min = 0;
    }
    int32_t x_max = field.x + img->height / 60;
    if(x_min > img->width - 1){
        x_min = img->width - 1;
    }
    int32_t y_min = field.y - img->height / 60;
    if(y_min < 0){
        y_min = 0;
    }
    int32_t y_max = field.y + img->height / 60;
    if(y_min > img->height - 1){
        y_min = img->height - 1;
    }
    printf("avg %d-%d|%d-%d\n", x_min, x_max, y_min, y_max);

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


void calibrate_colors(const camera_fb_t* const img, const vec2di_t fields[8][8], color_calibration_t* const calibration){
    printf("Calibrating colors...\n");
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
}

field_state_t get_field_state(const camera_fb_t* const img, const vec2di_t field, bool black_field, const color_calibration_t* const calibration){

    rgb_color_t field_color = avg_color_field(img, field);
    //DEBUG printf("%d (%d|%d)\n", black_field, field.x, field.y);
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
    //DEBUG printf("min_distance %d\n", min_distance);
    //if(min_distance < 40){
        return state;
    //}
    return FIELD_UNKNOWN;
}