/*----------includes----------*/
#include "esp_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "countdown.h"
#include "esp_intr_alloc.h"
/*----------------------------*/
/*----------macros----------*/
#define STACK_SIZE              512
#define TIME_PER_PLAYER_IN_MS   1800000
#define STOPTIME_IN_S           3
#define STOPTIME_IN_MS          STOPTIME_IN_S * 1000
#define CCT_TICKS_PER_US 	    CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ
#define PUSH_PIN                16
#define ESP_INTR_FLAG_DEFAULT   0
/*--------------------------*/
/*----------globals----------*/
bool isGameStarted = true; // TODO make game state struct
bool itsPlayerOnesTurn = true;
static xQueueHandle gpio_evt_queue = NULL;
/*---------------------------*/

void countdown_init(){
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[PUSH_PIN], PIN_FUNC_GPIO);
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = 1ULL << PUSH_PIN;
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf); 

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PUSH_PIN, changePlayer, (void*) PUSH_PIN);

    xTaskCreate(
        countdown,
        "countdown",
        STACK_SIZE,
        NULL,
        ESP_TASK_PRIO_MAX - 2,
        NULL
    );
}

void countdown(void* pvParameters)
{
    /*----------locals----------*/
    int time_player1 = TIME_PER_PLAYER_IN_MS, time_player2 = TIME_PER_PLAYER_IN_MS;
    uint32_t cctMarker = 0, now = 0, exactlyTimeInMS = 0;
    const TickType_t xDelay = 1 / portTICK_PERIOD_MS;
    /*--------------------------*/
    while (1)
    {
        if (itsPlayerOnesTurn)
        {
            time_player1 -= exactlyTimeInMS;
            printf("Time Player 1: %i\n", time_player1);
        }
        else
        {
            time_player2 -= exactlyTimeInMS;
            printf("Time Player 2: %i\n", time_player2);
        }

        if (time_player1 <= 0)
        {
            printf("Player 1 lost");
            isGameStarted = false;
        }
        else if (time_player2 <= 0)
        {
            printf("Player 2 lost");
            isGameStarted = false;
        }

        cctMarker = xthal_get_ccount();
        vTaskDelay(xDelay);
        now = xthal_get_ccount();

        exactlyTimeInMS = (cctMarker <= now ?
            ((now - cctMarker)/CCT_TICKS_PER_US) :
            ((now + (0xFFFFFFFF - cctMarker))/CCT_TICKS_PER_US))/1000;

        // if (cctMarker <= now)
        //     exactlyTimeInMS = (((now - cctMarker)+CCT_TICKS_PER_US/2)/CCT_TICKS_PER_US)/1000;
        // else
        //     exactlyTimeInMS = ((now + (0xFFFFFFFF - cctMarker) + CCT_TICKS_PER_US/2)/CCT_TICKS_PER_US)/1000;
        
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
        if(exactlyTimeInMS < 50){
            return;
        }else if(exactlyTimeInMS < STOPTIME_IN_MS){
            itsPlayerOnesTurn = !itsPlayerOnesTurn;
        }else{
            isGameStarted = false;
        }
    }
}
