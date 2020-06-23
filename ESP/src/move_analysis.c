/*----------includes----------*/
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include "move_analysis.h"
#include "storage.h"
/*----------------------------*/
void analyse_move(field_state_change_t state[8][8])
{
    /*----------locals----------*/
    bool isPieceBeforeEmpty = false;
    char piece = ' ', fieldLetter, temp[LOGLINE_BUF_SIZE] = "", logline_buf[LOGLINE_BUF_SIZE] = "";
    int pieceField[2] = { 0, 0 }, changes = 0;
	static char completeField[8][8] =
	{
		{'T','S','L','D','K','L','S','T'},
		{'B','B','B','B','B','B','B','B'},
		{'0','0','0','0','0','0','0','0'},
		{'0','0','0','0','0','0','0','0'},
		{'0','0','0','0','0','0','0','0'},
		{'0','0','0','0','0','0','0','0'},
		{'B','B','B','B','B','B','B','B'},
		{'T','S','L','D','K','L','S','T'}
	};
    /*--------------------------*/
    /*----------clear buffer----------*/
    memset(logline_buf, '\0', LOGLINE_BUF_SIZE);
    /*--------------------------------*/
    for (int y = 0, j = 8; y < 8; y++, j--)
	{
		for (int x = 0; x < 8; x++)
		{
			/*----------change number into a letter----------*/
			fieldLetter = 'A' + x;
			/*-----------------------------------------------*/
			if (FIELD_CHANGE_NO_CHANGE == state[y][x])
				continue;
			else if (FIELD_CHANGE_EMPTY == state[y][x])
			{
				changes++;
				piece = completeField[y][x];

				if (isPieceBeforeEmpty)
				{
					completeField[pieceField[0]][pieceField[1]] = piece;
					isPieceBeforeEmpty = false;
				}

				if ('B' == piece)
					memset(logline_buf, '\0', LOGLINE_BUF_SIZE);
				else
					logline_buf[0] = piece;

				logline_buf[strlen(logline_buf)] = fieldLetter;
				logline_buf[strlen(logline_buf)] = j + '0';
				memcpy(logline_buf + strlen(logline_buf), temp, strlen(temp));

				completeField[y][x] = '0';
			}
			else if ((FIELD_CHANGE_WHITE_PIECE == state[y][x]) || (FIELD_CHANGE_BLACK_PIECE == state[y][x]))
			{
				changes++;
				if (!strlen(logline_buf))
				{
					if ('0' == completeField[y][x])
						temp[0] = '-';
					else
						temp[0] = 'X';

					temp[strlen(temp)] = fieldLetter;
					temp[strlen(temp)] = j + '0';
					pieceField[0] = y;
					pieceField[1] = x;
					isPieceBeforeEmpty = true;
				}
				else
				{
					if ('0' == completeField[y][x])
						logline_buf[strlen(logline_buf)] = '-';
					else
						logline_buf[strlen(logline_buf)] += 'X';

					logline_buf[strlen(logline_buf)] = fieldLetter;
					logline_buf[strlen(logline_buf)] = j + '0';
					completeField[y][x] = piece;
					isPieceBeforeEmpty = false;
				}
			}
		}
	}

    if (changes >= 3)
	{
		memset(logline_buf, '\0', LOGLINE_BUF_SIZE);
		changes = 0;
		isCastling(state, logline_buf, completeField);
	}
    
    /*----------write file----------*/
	FILE *fp;
	fp = fopen("/sdcard/chess_notation.txt", "a");

	if (NULL == fp)
		printf("file couldn't be opened");
	else
		fprintf(fp, "%s\n", logline_buf);
	fclose(fp);
	/*------------------------------*/
}

void isCastling(field_state_change_t state[8][8], char* const logline_buf, char completeField[8][8])
{
	if ((FIELD_CHANGE_EMPTY == state[0][0]) && (FIELD_CHANGE_BLACK_PIECE == state[0][2]) &&
		(FIELD_CHANGE_BLACK_PIECE == state[0][3]) && (FIELD_CHANGE_EMPTY == state[0][4]))
	{
		completeField[0][0] = '0';
		completeField[0][2] = 'K';
		completeField[0][3] = 'T';
		completeField[0][4] = '0';
		memcpy(logline_buf, "0-0-0", 5);
	}
	else if ((FIELD_CHANGE_EMPTY == state[7][0]) && (FIELD_CHANGE_WHITE_PIECE == state[7][2]) &&
		(FIELD_CHANGE_WHITE_PIECE == state[7][3]) && (FIELD_CHANGE_WHITE_PIECE == state[7][4]))
	{
		completeField[7][0] = '0';
		completeField[7][2] = 'K';
		completeField[7][3] = 'T';
		completeField[7][4] = '0';
		memcpy(logline_buf, "0-0-0", 5);
	}
	else if ((FIELD_CHANGE_EMPTY == state[0][4]) && (FIELD_CHANGE_BLACK_PIECE == state[0][5]) &&
		(FIELD_CHANGE_BLACK_PIECE == state[0][6]) && (FIELD_CHANGE_EMPTY == state[0][7]))
	{
		completeField[0][4] = '0';
		completeField[0][5] = 'T';
		completeField[0][6] = 'K';
		completeField[0][7] = '0';
		memcpy(logline_buf, "0-0", 3);
	}
	else if ((FIELD_CHANGE_EMPTY == state[7][4]) && (FIELD_CHANGE_WHITE_PIECE == state[7][5]) &&
		(FIELD_CHANGE_WHITE_PIECE == state[7][6]) && (FIELD_CHANGE_EMPTY == state[7][7]))
	{
		completeField[7][4] = '0';
		completeField[7][5] = 'T';
		completeField[7][6] = 'K';
		completeField[7][7] = '0';
		memcpy(logline_buf, "0-0", 3);
	}
}