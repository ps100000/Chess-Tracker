#ifndef _MOVE_ANALYSIS_H_
#define _MOVE_ANALYSIS_H_
/*----------includes----------*/
#include <stdio.h>
/*----------------------------*/
/*----------macros----------*/
#define LOGLINE_BUF_SIZE    10
/*--------------------------*/
/*----------globals----------*/
typedef enum field_state_change
{
    FIELD_NO_CHANGE,
    FIELD_EMPTY,
    FIELD_WHITE_PIECE,
    FIELD_BLACK_PIECE
} field_state_change_t;
/*---------------------------*/
/*----------functions----------*/
void isCastling(field_state_change_t state[8][8], char* const notationString, char completeField[8][8]);
void analyse_move(field_state_change_t stateChange[8][8], char* logline_buf);
/*-----------------------------*/
#endif