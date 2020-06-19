/*----------includes----------*/
#include <FreeRTOS.h>
#include <task.h>
#include "..\include\countdown.h"
/*----------------------------*/
/*----------macros----------*/
#define STACK_SIZE      512
#define TIME_PER_PLAYER 1800000
#define PUSH_PIN        16
#define HIGH            1
#define LOW             0
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
/*---------------------------*/
/*----------init----------*/
gpio_set_direction(PUSH_PIN,GPIO_MODE_INPUT);
attachInterrupt(PUSH_PIN, changePlayer, HIGH);
/*------------------------*/
void timer()
{
    /*----------locals----------*/
    const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
    int time_player1 = TIME_PER_PLAYER, time_player2 = TIME_PER_PLAYER;
    bool itsPlayerOnesTurn = true;
    /*--------------------------*/
    while (1)
    {
        if (itsPlayerOnesTurn)
        {
            time_player1 -= xDelay;
            printf("%i", time_player1);
        }
        else
        {
            time_player2 -= xDelay;
            printf("%i", time_player2);
        }

        vTaskDelay(xDelay);
    }
}

void changePlayer()
{
    /*---???---*/
        static int PushPinLastState = LOW;
        int PushPinCurrentState = digitalRead(PUSH_PIN);

        if (PushPinLastState != PushPinCurrentState)
        {
            PushPinLastState = PushPinCurrentState;

            if (HIGH == digitalRead(PUSH_PIN))
                itsPlayerOnesTurn = !itsPlayerOnesTurn;
        }
        /*---???---*/
}
