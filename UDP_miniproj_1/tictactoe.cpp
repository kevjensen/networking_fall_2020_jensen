//
// Created by kali on 10/17/20.
//

#include <iostream>
#include "tictactoe.h"

// Kept getting undefined references late in the linking process no matter what return type I used or how I structured
// the function.
void game_winner(struct Games &game) {

	//Rows
	if (game.board[0] == 'X' && game.board[1] == 'X' && game.board[2] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[3] == 'X' && game.board[4] == 'X' && game.board[5] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[6] == 'X' && game.board[7] == 'X' && game.board[8] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[0] == 'O' && game.board[1] == 'O' && game.board[2] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	} else if (game.board[3] == 'O' && game.board[4] == 'O' && game.board[5] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	} else if (game.board[6] == 'O' && game.board[7] == 'O' && game.board[8] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	}

	//Columns
	else if (game.board[0] == 'X' && game.board[3] == 'X' && game.board[6] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[1] == 'X' && game.board[4] == 'X' && game.board[7] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[0] == 'X' && game.board[3] == 'X' && game.board[6] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[0] == 'O' && game.board[3] == 'O' && game.board[6] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	} else if (game.board[1] == 'O' && game.board[4] == 'O' && game.board[7] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	} else if (game.board[2] == 'O' && game.board[5] == 'O' && game.board[8] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	}

	//Diagonals
	else if (game.board[0] == 'X' && game.board[4] == 'X' && game.board[8] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[2] == 'X' && game.board[4] == 'X' && game.board[6] == 'X') {
		std::cout << "X is a winner!\n";
		//return game.result = X_WIN;
	} else if (game.board[0] == 'O' && game.board[4] == 'O' && game.board[8] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	} else if (game.board[2] == 'O' && game.board[4] == 'O' && game.board[6] == 'O') {
		std::cout << "O is a winner!\n";
		//return game.result = O_WIN;
	} else {
		std::cout << "Cat's game.\n";
		//return game.result = CATS_GAME;
	}
}