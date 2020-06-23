#ifndef _IMG_ANALYSIS_PIECES_H_
#define _IMG_ANALYSIS_PIECES_H_

#include "color.h"
#include "vec2d.h"
#include "esp_camera.h"

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

void calibrate_colors(const camera_fb_t* const img, const vec2di_t fields[8][8], color_calibration_t* const calibration);
field_state_t get_field_state(const camera_fb_t* const img, const vec2di_t field, bool black_field, const color_calibration_t* const calibration);
rgb_color_t avg_color_field(const camera_fb_t* const img, const vec2di_t field);

#endif
