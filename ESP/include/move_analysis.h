#ifndef _MOVE_ANALYSIS_H_
#define _MOVE_ANALYSIS_H_
/*----------includes----------*/
#include <stdio.h>
/*----------------------------*/
/*----------macros----------*/
#define LOGLINE_BUF_SIZE    10
/*--------------------------*/
/*----------globals----------*/
int pieceField[2];
char completeField[8][8];
typedef enum field_state_change
{
    FIELD_CHANGE_NO_CHANGE,
    FIELD_CHANGE_EMPTY,
    FIELD_CHANGE_WHITE_PIECE,
    FIELD_CHANGE_BLACK_PIECE
} field_state_change_t;
/*---------------------------*/
/*----------functions----------*/
void isCastling(field_state_change_t state[8][8], char* const notationString, char completeField[8][8]);
void analyse_move(field_state_change_t stateChange[8][8]);
/*-----------------------------*/
#endif