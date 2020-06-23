/*----------includes----------*/
#include <esp_task.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <xtensa/core-macros.h>
#include <esp_intr_alloc.h>

#include "countdown.h"
#include "img_analysis.h"
#include "esp_intr_alloc.h"
#include "move_analysis.h"
/*----------------------------*/
/*----------macros----------*/
#define STACK_SIZE              2048
#define TIME_PER_PLAYER_IN_MS   1800000
#define STOPTIME_IN_S           3
#define STOPTIME_IN_MS          STOPTIME_IN_S * 1000
#define CCT_TICKS_PER_US 	    CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ
#define PUSH_PIN                4
#define ESP_INTR_FLAG_DEFAULT   0
#define IC_RESET_PIN            12   // evtl. ändern
#define IC_CLOCK_PIN            33   // evtl. ändern
#define NO_OF_LEDS              6
/*--------------------------*/
/*----------globals----------*/
bool isGameStarted = true; // TODO make game state struct
uint16_t turn = 0;
bool itsPlayerWhitesTurn = true;
bool LEDs[NO_OF_LEDS];
char promotionPieces[4] = { 'D', 'T', 'L', 'S' };
int promotionCounter = 0;
static xQueueHandle gpio_evt_queue = NULL;
static const char* TAG = "countdown";
/*---------------------------*/

void countdown_init(){
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[IC_RESET_PIN], PIN_FUNC_GPIO);
    gpio_set_direction(IC_RESET_PIN, GPIO_MODE_OUTPUT);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[IC_CLOCK_PIN], PIN_FUNC_GPIO);
    gpio_set_direction(IC_CLOCK_PIN, GPIO_MODE_OUTPUT);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[PUSH_PIN], PIN_FUNC_GPIO);
    gpio_set_direction(PUSH_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PUSH_PIN, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(PUSH_PIN, GPIO_PIN_INTR_ANYEDGE);
    gpio_intr_enable(PUSH_PIN);
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    gpio_isr_handler_add(PUSH_PIN, changePlayer, (void*) PUSH_PIN);

    xTaskCreate(
        countdown_task,
        "countdown",
        STACK_SIZE,
        NULL,
        ESP_TASK_PRIO_MAX - 2,
        NULL
    );

    xTaskCreate(
        blinkLED_task,
        "blinkLED",
        STACK_SIZE,
        NULL,
        ESP_TASK_PRIO_MAX - 2,
        NULL
    );

    LEDs[0] = true;
    for (int i = 1; i < NO_OF_LEDS; i++){
        LEDs[i] = false;
    }
}

void countdown_task(void* pvParameters)
{
    /*----------locals----------*/
    int time_playerWhite = TIME_PER_PLAYER_IN_MS, time_playerBlack = TIME_PER_PLAYER_IN_MS;
    uint32_t cctMarker = 0, now = 0, exactlyTimeInMS = 0;
    static const TickType_t xDelay = 9000 / portTICK_PERIOD_MS;
    ESP_LOGI(TAG, "delay: %d\n", xDelay);
    /*--------------------------*/
    while (1)
    {
        if (itsPlayerWhitesTurn)
        {
            time_playerWhite -= exactlyTimeInMS;
            ESP_LOGI(TAG, "Time Player 1: %i\n", time_playerWhite);
        }
        else
        {
            time_playerBlack -= exactlyTimeInMS;
            ESP_LOGI(TAG, "Time Player 2: %i\n", time_playerBlack);
        }

        if (time_playerWhite <= 0)
        {
            ESP_LOGI(TAG, "Player 1 lost");
            isGameStarted = false;
        }
        else if (time_playerBlack <= 0)
        {
            ESP_LOGI(TAG, "Player 2 lost");
            isGameStarted = false;
        }

        cctMarker = XTHAL_GET_CCOUNT();
        vTaskDelay(xDelay);
        now = XTHAL_GET_CCOUNT();
        ESP_LOGI(TAG, "turn %d\n",turn);

        exactlyTimeInMS = (cctMarker <= now ?
            ((now - cctMarker)/CCT_TICKS_PER_US) :
            ((now + (0xFFFFFFFF - cctMarker))/CCT_TICKS_PER_US))/1000;        
    }
}

void IRAM_ATTR changePlayer(void* arg)
{
    static uint32_t cctMarker = 0;
    if(gpio_get_level(PUSH_PIN)){
        cctMarker = xthal_get_ccount();
    }else{
        uint32_t now = xthal_get_ccount();
        uint32_t exactlyTimeInMS = (cctMarker <= now ?
            ((now - cctMarker)/CCT_TICKS_PER_US) :
            ((now + (0xFFFFFFFF - cctMarker))/CCT_TICKS_PER_US))/1000;
        if(exactlyTimeInMS < 250){
            return;
        }else if(exactlyTimeInMS < STOPTIME_IN_MS){
            turn++;
            itsPlayerWhitesTurn = !itsPlayerWhitesTurn;
            LEDs[0] = !LEDs[0];
            LEDs[1] = !LEDs[1];
        }else{
            isGameStarted = false;
        }
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR( img_analysis_taskhandle, &xHigherPriorityTaskWoken );
        portYIELD_FROM_ISR();
    }
}

void IRAM_ATTR promotion(void* arg)
{
    static uint32_t cctMarker = 0;
    if(gpio_get_level(PUSH_PIN)){
        cctMarker = xthal_get_ccount();
    }else{
        uint32_t now = xthal_get_ccount();
        uint32_t exactlyTimeInMS = (cctMarker <= now ?
            ((now - cctMarker)/CCT_TICKS_PER_US) :
            ((now + (0xFFFFFFFF - cctMarker))/CCT_TICKS_PER_US))/1000;
        if(exactlyTimeInMS < 50){
            return;
        }else if(exactlyTimeInMS < STOPTIME_IN_MS){
            LEDs[promotionCounter+2] = !LEDs[promotionCounter+2];
            promotionCounter++;
            promotionCounter %= 4;
            LEDs[promotionCounter+2] = !LEDs[promotionCounter+2];
        }else{
            //confirm = true;
            completeField[pieceField[0]][pieceField[1]] = promotionPieces[promotionCounter];
            gpio_isr_handler_remove(PUSH_PIN);
            gpio_isr_handler_add(PUSH_PIN, changePlayer, (void*) PUSH_PIN);
        }
    }
}

void blinkLED_task()
{
    while(1){
        uint8_t num_on = 0;
        gpio_set_level(IC_CLOCK_PIN, 1);
        ets_delay_us(1);
        gpio_set_level(IC_CLOCK_PIN, 0);
        for (int i = 0; i < NO_OF_LEDS; i++){
            gpio_set_level(IC_CLOCK_PIN, 1);
            ets_delay_us(1);
            gpio_set_level(IC_CLOCK_PIN, 0);
            if (LEDs[i]){
                num_on++;
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }else{
                ets_delay_us(1);
            }
        }
        gpio_set_level(IC_RESET_PIN, 1);
        ets_delay_us(1);
        gpio_set_level(IC_RESET_PIN, 0);
        vTaskDelay((NO_OF_LEDS - num_on) * 250 / portTICK_PERIOD_MS + 1);
    }
}