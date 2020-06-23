
#include "esp_camera.h"
#include <esp_http_server.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp32/spiram.h>
#include <driver/uart.h>

#include "countdown.h"
#include "storage.h"

static const char* TAG = "cam";

//WROVER-KIT PIN Map
#define CAM_PIN_PWDN    32 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB888,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_SVGA,//QQVGA-QXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 1 //if more than one, i2s runs in continuous mode. Use only with JPEG
};

esp_err_t camera_module_init(){
    //power up the camera if PWDN pin is defined
    if(CAM_PIN_PWDN != -1){
        PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CAM_PIN_PWDN], PIN_FUNC_GPIO);
        gpio_set_direction(CAM_PIN_PWDN,GPIO_MODE_OUTPUT);
        gpio_set_level(CAM_PIN_PWDN, 0);
    }

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

void process_image(size_t width, size_t height, pixformat_t format, uint8_t* buf, size_t len){
    size_t count = 0;
    static const char img[4] = {' ', '-', '+', '#'};
    ESP_LOGI(TAG, "width %d height %d format %d len %d", width, height, format, len);

    //ESP_LOG_BUFFER_HEX_LEVEL("", buf,len, ESP_LOG_INFO);
    for (size_t y = 0; y < height; y += 4){
        for (size_t x = 0; x < width; x += 4){
            printf("%c", img[buf[(y * width + x) * 3] / 65]);
            count++;

            //red += buf[count * 3];
            //green += buf[count * 3 + 1];
            //blue += buf[count * 3 + 2];
            
            //uint32_t index = (y * width + x) *2;
            //printf("0x%x,0x%x,0x%x, ", buf[index] & 0b11111, ((buf[index] >> 5) & 0b111) | (buf[index + 1] & 0b111), buf[index + 1] >> 3 & 0b11111);
        }
        if(!(y % 20))
            vTaskDelay(1);
        printf("\n");
    }
    printf("\n");
    for (size_t y = 0; y < height; y += 4){
        for (size_t x = 0; x < width; x += 4){
            printf("%c", img[buf[(y * width + x) * 3 + 1] / 65]);
            count++;
        }
        if(!(y % 20))
            vTaskDelay(1);
        printf("\n");
    }
    printf("\n");
    for (size_t y = 0; y < height; y += 4){
        for (size_t x = 0; x < width; x += 4){
            printf("%c", img[buf[(y * width + x) * 3 + 2] / 65]);
            count++;
        }
        if(!(y % 20))
            vTaskDelay(1);
        printf("\n");
    }
    printf("\n %d\n", count);
}

esp_err_t camera_capture(){
    //acquire a frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }
    //replace this with your own function
    process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);
  
    //return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    return ESP_OK;
}

esp_err_t bmp_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    uint8_t * buf = NULL;
    size_t buf_len = 0;
    bool converted = frame2bmp(fb, &buf, &buf_len);
    esp_camera_fb_return(fb);
    if(!converted){
        ESP_LOGE(TAG, "BMP conversion failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    res = httpd_resp_set_type(req, "image/x-windows-bmp")
       || httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.bmp")
       || httpd_resp_send(req, (const char *)buf, buf_len);
    free(buf);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "BMP: %uKB %ums", (uint32_t)(buf_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}

void app_main() {
    camera_module_init();
    //for (size_t i = 0; i < 20; i++){
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    
    countdown_init();
    storage_init();

    printf("%d\n", esp_get_free_heap_size());
    while(1){
        camera_capture();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    sensor_t * s = esp_camera_sensor_get();
    // TODO change settings

}
