#ifndef _IMG_ANALYSIS_BOARD_H_
#define _IMG_ANALYSIS_BOARD_H_

#include "esp_camera.h"
#include "vec2d.h"

#define QUAD_COLOR_SUM(getter, img, x, y, offset, color_res)  ((getter(img, x, y) & (~0u << (8 - color_res))) + \
                                                    (getter(img, x + offset, y) & (~0u << (8 - color_res))) + \
                                                    (getter(img, x, y + offset) & (~0u << (8 - color_res))) + \
                                                    (getter(img, x + offset, y + offset) & (~0u << (8 - color_res))))
#define MARKER_COLOR_RES 2

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

bool check_possible_marker(const camera_fb_t* const img, cmarker_t* const marker);
bool find_marker(const camera_fb_t* const img, cmarker_t* marker, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
bool find_markers(const camera_fb_t* const img, cmarker_t markers[4]);
void calculate_fields(const camera_fb_t* const img, vec2di_t fields[8][8], const cmarker_t markers[4]);
cmarker_check_result_t check_markers(const camera_fb_t* const img, cmarker_t markers[4]);

#endif
