#!/usr/bin/python3
# -*- coding: utf-8 -*-
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(message)s',
)

LOG = logging.getLogger(__name__)

import os
import sys
import chess
import chess.pgn
import chess.engine
import copy
import PySimpleGUI as sg
import re

from time import sleep

resources = './resources/'

BLANK = 0
PAWNB = 1
ROOKB = 2
BISHOPB = 3
KNIGHTB = 4
KINGB = 5
QUEENB = 6
PAWNW = 7
ROOKW = 8
BISHOPW = 9
KNIGHTW = 10
KINGW = 11
QUEENW = 12

images = {
BLANK: os.path.join(resources, 'blank.png'),
PAWNB: os.path.join(resources, 'pawn_black.png'),
ROOKB: os.path.join(resources, 'rook_black.png'),
BISHOPB: os.path.join(resources, 'bishop_black.png'),
KNIGHTB: os.path.join(resources, 'knight_black.png'),
KINGB: os.path.join(resources, 'king_black.png'),
QUEENB: os.path.join(resources, 'queen_black.png'),
PAWNW: os.path.join(resources, 'pawn_white.png'),
ROOKW: os.path.join(resources, 'rook_white.png'),
BISHOPW: os.path.join(resources, 'bishop_white.png'),
KNIGHTW: os.path.join(resources, 'knight_white.png'),
KINGW: os.path.join(resources, 'king_white.png'),
QUEENW: os.path.join(resources, 'queen_white.png')
}

initial_board = [[ROOKB, KNIGHTB, BISHOPB, QUEENB, KINGB, BISHOPB, KNIGHTB, ROOKB],
[PAWNB, ] * 8,
[BLANK, ] * 8,
[BLANK, ] * 8,
[BLANK, ] * 8,
[BLANK, ] * 8,
[PAWNW, ] * 8,
[ROOKW, KNIGHTW, BISHOPW, QUEENW, KINGW, BISHOPW, KNIGHTW, ROOKW]]

def render_square(image, key, location):
    if (location[0] + location[1]) % 2:
        color = '#B58863'
    else:
        color = '#F0D9B5'
    return sg.Button('', image_filename=image, size=(1, 1), button_color=('white', color), pad=(0, 0), key=key, focus=False)

def read_window_break_if_closed(engine):
    button, value = window.Read()
    if button == sg.WIN_CLOSED:
        # if user closes window
        engine.quit()
        sys.exit()
    return button, value

def redraw_board(window, board):
    for i in range(8):
        for j in range(8):
            color = '#B58863' if (i+j) % 2 else '#F0D9B5'
            piece_image = images[board[i][j]]
            elem = window.FindElement(key=(i,j))
            elem.Update(button_color = ('white', color),
                        image_filename=piece_image,)

def create_gui(board):
    sg.ChangeLookAndFeel('GreenTan')
    # create initial board setup
    psg_board = copy.deepcopy(initial_board)
    # the main board display layout
    board_layout = [[sg.T('     ')] + [sg.T('{}'.format(a), pad=((40, 40), 0), font='Any 13') for a in 'abcdefgh']]
    # loop through board and create buttons with images
    for i in range(8):
        row = [sg.T(str(8 - i) + '   ', font='Any 13')]
        for j in range(8):
            piece_image = images[psg_board[i][j]]
            row.append(render_square(piece_image, key=(i, j), location=(i, j)))
        row.append(sg.T(str(8 - i) + '   ', font='Any 13'))
        board_layout.append(row)

    # add the labels across bottom of board
    board_layout.append([sg.T('     ')] + [sg.T('{}'.format(a), pad=((40, 40), 0), font='Any 13') for a in 'abcdefgh'])

    # setup the controls on the right side of screen
    board_controls = [[sg.RButton('Continue', key='Continue')],
                    [sg.Text('Move List')],
                    [sg.Multiline([], do_not_clear=True, autoscroll=True, size=(30, 30), key='_movelist_')],
                    [sg.Text('Boardscore')],
                    [sg.Multiline([], do_not_clear=False, size=(30, 10), key='_score_')],
                    [sg.Text('best possible move from stockfish:')],
                    [sg.Multiline([], do_not_clear=False, size=(30, 10), key='_best_move_')]
                    ]

    board_tab = [[sg.Column(board_layout)]]

    # the main window layout
    layout = [[sg.TabGroup([[sg.Tab('Board',board_tab)]], title_color='red'),
            sg.Column(board_controls)]]


    window = sg.Window('Chess-Tracker-GUI',
                    default_button_element_size=(12, 1),
                    auto_size_buttons=False
                    ).Layout(layout)

    return psg_board, window

def make_move_gui(move_str, psg_board, window, piece=''):
    from_col = ord(move_str[0]) - ord('a')
    from_row = 8 - int(move_str[1])
    to_col = ord(move_str[2]) - ord('a')
    to_row = 8 - int(move_str[3])

    if not piece:
       piece = psg_board[from_row][from_col]
    psg_board[from_row][from_col] = BLANK
    psg_board[to_row][to_col] = piece
    redraw_board(window, psg_board)

def castling_short(move_str, psg_board, window):
    if board.turn:
        move_str = 'e1h1'
        make_move_gui('e1g1', psg_board, window)
        make_move_gui('h1f1', psg_board, window)
    else:
        move_str = 'e8h8'
        make_move_gui('e8g8', psg_board, window)
        make_move_gui('h8f8', psg_board, window)
    return move_str

def castling_long(move_str, psg_board, window):
    if board.turn:
        move_str = 'e1a1'
        make_move_gui('e1c1', psg_board, window)
        make_move_gui('a1d1', psg_board, window)
    else:
        move_str = 'e8a8'
        make_move_gui('e8c8', psg_board, window)
        make_move_gui('a8d8', psg_board, window)
    return move_str

def promotion(move_str, psg_board, window):
    wished_piece = move_str[5]
    if wished_piece == 'D':
        move_str = move_str[0:4] + 'q'
        if board.turn:
            make_move_gui('move_str', psg_board, window, piece=QUEENW)
        else:
            make_move_gui('move_str', psg_board, window, piece=QUEENB)
    elif wished_piece == 'S':
        move_str = move_str[0:4] + 'k'
        if board.turn:
            make_move_gui('move_str', psg_board, window, piece=KNIGHTW)
        else:
            make_move_gui('move_str', psg_board, window, piece=KNIGHTB)
    elif wished_piece == 'T':
        move_str = move_str[0:4] + 'r'
        if board.turn:
            make_move_gui('move_str', psg_board, window, piece=ROOKW)
        else:
            make_move_gui('move_str', psg_board, window, piece=ROOKB)
    elif wished_piece == 'L':
        move_str = move_str[0:4] + 'b'
        if board.turn:
            make_move_gui('move_str', psg_board, window, piece=BISHOPW)
        else:
            make_move_gui('move_str', psg_board, window, piece=BISHOPB)
    return move_str

def make_move(psg_board, window, move_str, line, engine):
    if move_str == '0-0':
        move_str = castling_short(move_str, psg_board, window)
    elif move_str == '0-0-0':
        move_str = castling_long(move_str, psg_board, window)
    # en passante
    elif 'ep' in move_str:
        if board.turn:
            move_str = move_str[0:4]
            enemy_pawn = move_str[2] + str(int(move_str[3]) - 1)
            make_move_gui(move_str[0:2] + enemy_pawn, psg_board, window)
            make_move_gui(enemy_pawn + move_str[2:4], psg_board, window)
        else:
            move_str = move_str[0:4]
            enemy_pawn = move_str[2] + str(int(move_str[3]) + 1)
            make_move_gui(move_str[0:2] + enemy_pawn, psg_board, window)
            make_move_gui(enemy_pawn + move_str[2:4], psg_board, window)
    elif '=' in move_str:
        move_str = promotion(move_str, psg_board, window)
    else:
        make_move_gui(move_str, psg_board, window)

    move = chess.Move.from_uci(move_str)

    if not (board.is_legal(move) != board.is_castling(move)):
        if board.turn:
            sg.Popup(f'ERROR: {line} is not a legal move from white.\n')
        else:
            sg.Popup(f'ERROR: {line} is not a legal move from black.\n')
        engine.quit()
        sys.exit()

    board.push(move)

    analyzed_score = engine.analyse(board, chess.engine.Limit(time=0.2))
    score = analyzed_score.get('score')
    depth = analyzed_score.get('depth')
    window.FindElement('_score_').Update(f'Score of your board is {score} with a analyzed depth of {depth}.', append=True)
    window.FindElement('_movelist_').Update(line + '\n', append=True)

    best_move = engine.play(board, chess.engine.Limit(time=0.2)).move
    window.FindElement('_best_move_').Update(f'{best_move} with a analyzed depth of {depth}.', append=True)

    if (board.is_checkmate()):
        sg.Popup('Checkmate!', board.result(), '. Thank you for playing')
        engine.quit()
        sys.exit()

    if (board.is_check()):
        sg.Popup('Check')

if __name__ == '__main__':

    try:
        os.path.exists(sys.argv[1])
    except IndexError:
        LOG.error('Please enter a chess logfile as argument.')
        sys.exit()
    except FileNotFoundError:
        LOG.error('Please enter an existing chess logfile as argument.')
        sys.exit()

    board = chess.Board()
    LOG.debug(f'START\n{board}\n\n')
    filename = sg.PopupGetFile('\n'.join(('To begin, set location of AI EXE file',
                                          'If you have not done so already, download the engine',
                                          'Download the StockFish Chess engine at: https://stockfishchess.org/download/')),
                               file_types=(('Chess AI Engine EXE File', '*.exe'),))
    engine = chess.engine.SimpleEngine.popen_uci(filename)
    # engine = chess.engine.SimpleEngine.popen_uci('C:/Users/Gerst/Desktop/Schulprojekt/stockfish-11-win/stockfish-11-win/Windows/stockfish_20011801_x64')

    contents = open(sys.argv[1], 'r').read()
    psg_board, window = create_gui(board)
    read_window_break_if_closed(engine)
    for line in re.findall(r'^(.*)$', contents, re.MULTILINE):
        regex_possible_moves = re.findall(r'(?:\D?(\D\d).(\D\d)\s?(ep)?(=[DSTL])?|(0-0))', line)
        # gives a uci chess string, eg. c2c4, regex gives tuple(c2)(c4)
        move = ''.join(regex_possible_moves[0])
        # results in a string instead of tuple
        make_move(psg_board, window, move, line, engine)
        LOG.debug(f'{board}\n')

        while True:
            button, value = read_window_break_if_closed(engine)
            if button == 'Continue':
                break
            else:
                sleep(1)

    sg.Popup('This was the last readable part. Thank you for playing!')
    engine.quit()
