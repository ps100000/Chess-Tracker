#include <freertos/FreeRTOS.h>
#include <esp_task.h>
#include <esp_log.h>
#include <freertos/task.h>

#include "img_analysis.h"
#include "img_analysis_board.h"
#include "img_analysis_pieces.h"
#include "vec2d.h"

static const char* TAG = "img_analysis";
TaskHandle_t img_analysis_taskhandle = NULL;

void img_analysis_init(){
    xTaskCreate(
        img_analysis_task,
        "img_analysis",
        2048,
        NULL,
        ESP_TASK_PRIO_MAX - 2,
        &img_analysis_taskhandle
    );
}

void img_analysis_task(void* pvParameters){
    vTaskDelay(2500 / portTICK_RATE_MS);
    uint32_t ulNotifiedValue;
    vec2di_t fields[8][8];
    field_state_t board_state[8][8] = {
        {FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE},
        {FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE, FIELD_WHITE_PIECE},
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {   FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   ,    FIELD_EMPTY   },
        {FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE},
        {FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE, FIELD_BLACK_PIECE}
    };
    color_calibration_t calibration;

    bool first = true;
    while(1){
        camera_fb_t* img = esp_camera_fb_get();
        for (size_t i = 0; i < img->len; i++){
            img->buf[i] &= 0b11000000;
        }
        if (!img) {
            ESP_LOGE(TAG, "Camera Capture Failed");
            return;
        }
        cmarker_t markers[4];
        if(find_markers(img, markers)){
            calculate_fields(img, fields, markers);
        }
        if(first){
            first = false;
            calibrate_colors(img, fields, &calibration);
            printf("white empty:(%d|%d|%d)\n", calibration.empty[0].red, calibration.empty[0].green, calibration.empty[0].blue);
            printf("black empty:(%d|%d|%d)\n", calibration.empty[1].red, calibration.empty[1].green, calibration.empty[1].blue);
            printf("white white_piece:(%d|%d|%d)\n", calibration.white_piece[0].red, calibration.white_piece[0].green, calibration.white_piece[0].blue);
            printf("black white_piece:(%d|%d|%d)\n", calibration.white_piece[1].red, calibration.white_piece[1].green, calibration.white_piece[1].blue);
            printf("white black_piece:(%d|%d|%d)\n", calibration.black_piece[0].red, calibration.black_piece[0].green, calibration.black_piece[0].blue);
            printf("black black_piece:(%d|%d|%d)\n", calibration.black_piece[1].red, calibration.black_piece[1].green, calibration.black_piece[1].blue);
        }
        esp_camera_fb_return(img);
        while(1){
            xTaskNotifyWait( 0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);
            img = esp_camera_fb_get();
            for (size_t i = 0; i < img->len; i++){
                img->buf[i] &= 0b11000000;
            }
            if (!img) {
                ESP_LOGE(TAG, "Camera Capture Failed");
                return;
            }
            cmarker_check_result_t check_marker_result = check_markers(img,markers);
            if(check_marker_result == CM_CHECK_ADJUSTED){
                printf("CM_CHECK_ADJUSTED\n");
                calculate_fields(img, fields, markers);
            }else if(check_marker_result == CM_CHECK_INVALID){
                printf("CM_CHECK_INVALID\n");
                esp_camera_fb_return(img);
                break;
            }else{
                printf("CM_CHECK_NO_CHANGE\n");
            }
            for (size_t y = 0; y < 8; y++){
                for (size_t x = 0; x < 8; x++){
                    field_state_t current_state = get_field_state(img, fields[x][y], !((x & 1) ^ (y & 1)), &calibration);
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
            esp_camera_fb_return(img);
        }
    }
}
