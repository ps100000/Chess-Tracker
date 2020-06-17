#ifndef _MOVE_ANALYSIS_H_
#define _MOVE_ANALYSIS_H_

#define LOGLINE_BUF_SIZE 10

enum field_state_change{
    FIELD_NO_CHANGE,
    FIELD_EMPTY,
    FIELD_WHITE_PIECE,
    FIELD_BLACK_PIECE
};

typedef enum field_state_change field_state_change_t;

void analyse_move(field_state_change_t state[8][8], char* logline_buf);

#endif
