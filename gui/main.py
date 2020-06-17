#!/usr/bin/python3
# -*- coding: utf-8 -*-
import logging

logging.basicConfig(
    level=logging.INFO,
    # %(asctime)s|%(name)s|%(levelname)s: 
    format='%(message)s',
)

LOG = logging.getLogger(__name__)

import chess
import chess.engine
import sys
import os
import re

if __name__ == '__main__':

    board = chess.Board()
    # engine = chess.engine.SimpleEngine.popen_uci("C:/Users/Gerst/Desktop/stockfish-11-win/Windows/stockfish_20011801_x64")

    if os.path.exists(sys.argv[1]):
        file = open(sys.argv[1], 'r')
        contents = file.read()
        move = ''
        LOG.info(f'START\n{board}\n\n')

        for move_tuple in re.findall(r'\D?(\D\d).(\D\d)', contents):
            # regex start and end of a move and gives the tuple eg. (c2, c4)
            move = ''.join(move_tuple)
            # gives a uci chess string, eg. c2c4
            # score_before = engine.analyse(board, chess.engine.Limit(time=0.2))['score']
            move = chess.Move.from_uci(move)

            if not board.is_legal(move):
                start = move_tuple[0]
                end = move_tuple[1]
                LOG.error(f'ERROR: This is not a legal move. Fom {start} to {end}\n')
                break

            board.push(move)
            LOG.info(f'{board}\n')

            # Lass die score anhand desjenigen bestimmen, für den sie gemacht ist chess.engine.PovScore(score_after, chess.Color)
            # score_after = engine.analyse(board, chess.engine.Limit(time=0.2))['score']
            # score = int(str(score_after)) - int(str(score_before))
            # if score > 0:
            #     LOG.info(f"This move was good. It raised your score about {score}")
            # if score < 0:
            #     LOG.info(f"This move was bad. It lowered your score about {score}")

            if (board.is_check()):
                LOG.info('Check')

            if (board.is_checkmate()):
                print('Checkmate\n')
                LOG.info(f'Checkmate\n{board.result}')
                break
            input('Press Enter to continue...\n')
        file.close

    # engine.quit()