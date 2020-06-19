/*----------includes----------*/
#include <FreeRTOS.h>
#include <task.h>
#include "..\include\countdown.h"
/*----------------------------*/
/*----------macros----------*/
#define STACK_SIZE              512
#define TIME_PER_PLAYER_IN_MS   1800000
#define STOPTIME_IN_S           3
#define STOPTIME_IN_MS          STOPTIME_IN_S * 1000
#define CCT_TICKS_PER_US 	    CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ
#define PUSH_PIN                16
#define HIGH                    1
#define LOW                     0
/*--------------------------*/
/*----------globals----------*/
TaskHandle_t xHandle = NULL;
BaseType_t xReturned = xCreateTask(
    timer,
    "countdown",
    STACK_SIZE,
    (void *) 1,
    tskIDLE_PRIORITY,
    &xHandle
);
bool isGameStarted = true;
/*---------------------------*/
/*----------init----------*/
gpio_set_direction(PUSH_PIN, GPIO_MODE_INPUT);
attachInterrupt(PUSH_PIN, changePlayer, HIGH);
/*------------------------*/
void timer()
{
    /*----------locals----------*/
    bool itsPlayerOnesTurn = true;
    int time_player1 = TIME_PER_PLAYER_IN_MS, time_player2 = TIME_PER_PLAYER_IN_MS;
    uint32_t cctMarker = 0, now = 0, exactlyTimeInMS = 0;
    const TickType_t xDelay = 1 / portTICK_PERIOD_MS;
    /*--------------------------*/
    while (1)
    {
        if (itsPlayerOnesTurn)
        {
            time_player1 -= exactlyTimeInMS;
            printf("%i\n", time_player1);
        }
        else
        {
            time_player2 -= exactlyTimeInMS;
            printf("%i\n", time_player2);
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

        cctMarker = XTHAL_GET_CCOUNT();

        vTaskDelay(xDelay);

        now = XTHAL_GET_CCOUNT();

        exactlyTimeInMS = (cctMarker <= now ?
            (((now - cctMarker)+CCT_TICKS_PER_US/2)/CCT_TICKS_PER_US)/1000 :
            ((now + (0xFFFFFFFF - cctMarker) + CCT_TICKS_PER_US/2)/CCT_TICKS_PER_US))/1000;

        // if (cctMarker <= now)
        //     exactlyTimeInMS = (((now - cctMarker)+CCT_TICKS_PER_US/2)/CCT_TICKS_PER_US)/1000;
        // else
        //     exactlyTimeInMS = ((now + (0xFFFFFFFF - cctMarker) + CCT_TICKS_PER_US/2)/CCT_TICKS_PER_US)/1000;
        
    }
}

void changePlayer()
{
    /*----------locals----------*/
    static int PushPinLastState = LOW;
    int PushPinCurrentState = digitalRead(PUSH_PIN), timeToStop = 0;
    /*--------------------------*/
    if (PushPinLastState != PushPinCurrentState)
    {
        PushPinLastState = PushPinCurrentState;

        if (HIGH == digitalRead(PUSH_PIN))
        {
            while(1)
            {
                if (LOW == digitalRead(PUSH_PIN))
                    itsPlayerOnesTurn = !itsPlayerOnesTurn;
                timeToStop++;
                if (STOPTIME_IN_MS == timeToStop)
                    isGameStarted = false;
                vTaskDelay(xDelay / 10);
            }
        }
    }
}
