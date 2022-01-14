# Othello AI
My tiny implementation of Othello/Reversi console game with AI player and MinMax algorithm (Alpha-Beta pruning).

### Commands
START < color > < time for move (2-30 sec) >
- starts game


MOVE < game state >
- program answers with best move
- game state: string, 64 characters, contains only 3 types: '-' represents empty spot, 'O' represents white coin, 'X' represents black coin.
- Example of initial configuration: ---------------------------OX------XO--------------------------- 


STOP
- immediately stops program executing

### Heuristic functions
Collection of several heuristics, calculates value of a board position. Choosed heuristics: mobility, coin parity,
utility value and corners-captured aspects of a board configuration. Each heuristic return value between -100 to 100 and these values
are weighted appropriately to play an optimal game. <br>
Heuristics inspiration:
[theory](https://courses.cs.washington.edu/courses/cse573/04au/Project/mini1/RUSSIA/Final_Paper.pdf),
[theory](https://kartikkukreja.wordpress.com/2013/03/30/heuristic-function-for-reversiothello/?fbclid=IwAR347nuWS4Rg5ioWZDwCpaGNxDWmCOIBoEZpxpxrHJRJuNyMzt-k9SNIJFM) and
[github](https://github.com/sadeqsheikhi/reversi_python_ai)
