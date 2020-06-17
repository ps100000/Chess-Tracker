/*----------includes----------*/
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "move_analysis.h"
/*----------------------------*/
/*----------globals----------*/
static char completeField[8][8] =
{
	'T','S','L','D','K','L','S','T',
	'B','B','B','B','B','B','B','B',
	'0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0',
	'0','0','0','0','0','0','0','0',
	'B','B','B','B','B','B','B','B',
	'T','S','L','D','K','L','S','T'
};
/*---------------------------*/
void analyse_move(field_state_change_t stateChange[8][8], char* logline_buf)
{
    /*----------locals----------*/
    bool isFigureBeforeEmpty = false;
    char piece = ' ', fieldLetter, notationString[] = "", temp[] = "";
    int pieceField[2] = { 0, 0 }, changes = 0;
    static field_state_change_t last_stateChange[8][8];
    /*--------------------------*/
    /*----------clear buffer----------*/
    memset(logline_buf, '\0', LOGLINE_BUF_SIZE);
    /*--------------------------------*/
    for (int i = 0, j = 8; i < 8; i++, j--)
	{
		for (int k = 0; k < 8; k++)
		{
			/*----------change number into a letter----------*/
			switch (k)
			{
				case 0:
				{
					fieldLetter = 'A';
					break;
				}
				case 1:
				{
					fieldLetter = 'B';
					break;
				}
				case 2:
				{
					fieldLetter = 'C';
					break;
				}
				case 3:
				{
					fieldLetter = 'D';
					break;
				}
				case 4:
				{
					fieldLetter = 'E';
					break;
				}
				case 5:
				{
					fieldLetter = 'F';
					break;
				}
				case 6:
				{
					fieldLetter = 'G';
					break;
				}
				case 7:
				{
					fieldLetter = 'H';
					break;
				}
				default:
				{
					break;
				}
			}
			/*-----------------------------------------------*/
			if (FIELD_NO_CHANGE == state[i][k])
				continue;
			else if (FIELD_EMPTY == state[i][k])
			{
				changes++;
				piece = completeField[i][k];

				if (true == isFigureBeforeEmpty)
				{
					completeField[pieceField[0]][pieceField[1]] = piece;
					isFigureBeforeEmpty = false;
				}

				if ('B' == piece)
                {
					notationString[] = ' ';
                }
				else
					notationString[] = piece;

				char buffer[2];
				_itoa_s(j, buffer, 2, 10);

				notationString[7] = notationString + fieldLetter + buffer[2] + temp[7];

				completeField[i][k] = '0';
			}
			else if ((FIELD_WHITE_PIECE == state[i][k]) || (FIELD_BLACK_PIECE == state[i][k]))
			{
				changes++;
				if ("" == notationString)
				{
					if ('0' == completeField[i][k])
						temp[7] = '-';
					else
						temp[7] = 'X';

					char buffer[2];
					_itoa_s(j, buffer, 2, 10);

					temp[7] = temp + fieldLetter + buffer;

					pieceField[0] = i;
					pieceField[1] = k;

					isFigureBeforeEmpty = true;
				}
				else
				{
					if ('0' == completeField[i][k])
						notationString[7] += '-';
					else
						notationString[7] += 'X';

					char buffer[2];
					_itoa_s(j, buffer, 2, 10);

					notationString[7] = notationString + fieldLetter + buffer;

					completeField[i][k] = piece;

					isFigureBeforeEmpty = false;
				}
			}
		}
	}

    if (changes >= 3)
	{
		notationString[7] = "";
		changes = 0;
		notationString[7] = isCastling(state, notationString);
	}
	/*else
		isCheck(state);*/
    
    // write file
	/*fstream f;
	f.open("chess_notation.txt", ios::app);
	f << notationString << "\n";
	f.close();*/

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf("%c", completeField[i][j]);
        }

        printf("\n");
    }
}

char isCastling(field_state_change_t stateChange[8][8], char notationString[7])
{
	if ((FIELD_EMPTY == state[0][0]) && (FIELD_BLACK_PIECE == state[0][2]) &&
		(FIELD_BLACK_PIECE == state[0][3]) && (FIELD_EMPTY == state[0][4]))
	{
		completeField[0][0] = '0';
		completeField[0][2] = 'K';
		completeField[0][3] = 'T';
		completeField[0][4] = '0';
		notationString = "0-0-0";
	}
	else if ((FIELD_EMPTY == state[7][0]) && (FIELD_WHITE_PIECE == state[7][2]) &&
		(FIELD_WHITE_PIECE == state[7][3]) && (FIELD_WHITE_PIECE == state[7][4]))
	{
		completeField[7][0] = '0';
		completeField[7][2] = 'K';
		completeField[7][3] = 'T';
		completeField[7][4] = '0';
		notationString = "0-0-0";
	}
	else if ((FIELD_EMPTY == state[0][4]) && (FIELD_BLACK_PIECE == state[0][5]) &&
		(FIELD_BLACK_PIECE == state[0][6]) && (FIELD_EMPTY == state[0][7]))
	{
		completeField[0][4] = '0';
		completeField[0][5] = 'T';
		completeField[0][6] = 'K';
		completeField[0][7] = '0';
		notationString = "0-0";
	}
	else if ((FIELD_EMPTY == state[7][4]) && (FIELD_WHITE_PIECE == state[7][5]) &&
		(FIELD_WHITE_PIECE == state[7][6]) && (FIELD_EMPTY == state[7][7]))
	{
		completeField[7][4] = '0';
		completeField[7][5] = 'T';
		completeField[7][6] = 'K';
		completeField[7][7] = '0';
		notationString = "0-0";
	}

	return notationString;
}

void isCheck(field_state_change_t stateChange[8][8])
{
	/*----------locals----------*/
	int chord[2] = { 0, 0 };
	/*--------------------------*/
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if ((FIELD_WHITE_PIECE == state[i][j]) || (FIELD_BLACK_PIECE == state[i][j]))
			{
				chord[0] = i;
				chord[1] = j;
			}
		}
	}

	switch (completeField[chord[0]][chord[1]])
	{
		case 'B':
		{
			break;
		}

		case 'T':
		{
			break;
		}

		case 'L':
		{
			break;
		}

		case 'S':
		{
			break;
		}

		case 'D':
		{
			break;
		}

		case 'K':
		{
			break;
		}

		default:
		{
			break;
		}
	}
}