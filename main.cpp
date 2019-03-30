//#include <stdlib.h>
//#include <stdio.h>

#include <iostream>
#include <vector>
#include <string>

#define CODECUP

#define BOARD_SPACES 64
#define NUMBER_OF_ROWS 8
#define NUMBER_OF_COLUMNS 8

#define BIT(i) (uint64_t(1) << (i))
#define BITPOSITION(xpos, ypos) BIT((xpos) + (ypos) * NUMBER_OF_ROWS)


#define STARTING_PIECES (BITPOSITION(3, 3) | BITPOSITION(4, 3) | BITPOSITION(3, 4) | BITPOSITION(4, 4))
#define STARTING_WHITE_PIECES (BITPOSITION(3, 3) | BITPOSITION(4, 4))
#define STARTING_ADJACENT_POSITIONS (\
		BITPOSITION(2, 2) | \
		BITPOSITION(3, 2) | \
		BITPOSITION(4, 2) | \
		BITPOSITION(5, 2) | \
		BITPOSITION(2, 3) | \
		BITPOSITION(5, 3) | \
		BITPOSITION(2, 4) | \
		BITPOSITION(5, 4) | \
		BITPOSITION(2, 5) | \
		BITPOSITION(3, 5) | \
		BITPOSITION(4, 5) | \
		BITPOSITION(5, 5) )

typedef unsigned int uint;
typedef uint64_t BitBoard;

#ifdef CODECUP
void assert(bool toTest, std::string message) {}
#else
void assert(bool toTest, std::string message) {
	if (!toTest) {
		std::cerr << message << std::endl;
		exit(-1);
	}
}
#endif

void printBitBoard(std::ostream& out, BitBoard b) {
	for(int iy = 0; iy < NUMBER_OF_ROWS; iy++) {
		for(int ix = 0; ix < NUMBER_OF_COLUMNS; ix++) {
			if((b & BITPOSITION(ix, iy)) != 0) {
				out << '1';
			} else {
				out << '0';
			}
		}
		out << std::endl;
	}
}

BitBoard getAdjacentPositions(int xpos, int ypos) {
	BitBoard adjacentPositions = 0;
	int allowLeft = 0;
	if (xpos > 0) {
		allowLeft = 1;
	}
	int allowRight = 0;
	if (xpos < (NUMBER_OF_COLUMNS-1)) {
		allowRight = 1;
	}
	int allowUp = 0;
	if (ypos > 0) {
		allowUp = 1;
	}
	int allowDown = 0;
	if (ypos < (NUMBER_OF_ROWS-1)) {
		allowDown = 1;
	}
	
	adjacentPositions |= (BITPOSITION(xpos-1, ypos) * allowLeft);
	adjacentPositions |= (BITPOSITION(xpos+1, ypos) * allowRight);
	adjacentPositions |= (BITPOSITION(xpos, ypos-1) * allowUp);
	adjacentPositions |= (BITPOSITION(xpos, ypos+1) * allowDown);
	
	adjacentPositions |= (BITPOSITION(xpos-1, ypos-1) * allowLeft * allowUp);
	adjacentPositions |= (BITPOSITION(xpos+1, ypos-1) * allowRight * allowUp);
	adjacentPositions |= (BITPOSITION(xpos-1, ypos+1) * allowLeft * allowDown);
	adjacentPositions |= (BITPOSITION(xpos+1, ypos+1) * allowRight * allowDown);
	
	return adjacentPositions;
}

struct Move {
	int xpos = 0;
	int ypos = 0;
	BitBoard bitsToFlip = 0;
};


class FlippoBoard {
	BitBoard _hasPiece = STARTING_PIECES;
	BitBoard _isWhitePiece = STARTING_WHITE_PIECES;
	BitBoard _adjacentPositions = STARTING_ADJACENT_POSITIONS;
	int _turn = 0;
	
public:
	bool isWhiteTurn() const {
		return (_turn % 2) == 0;
	}
	
	bool isFinished() const {
		return (~_hasPiece == 0);
	}
	
	int whitePlayerScore() const {
		int count = 0;
		for(int ii = 0; ii < BOARD_SPACES; ii++) {
			if ((_isWhitePiece & BIT(ii)) != 0) {
				count++;
			}
		}
		return count - 2;
	}
	
	int blackPlayerScore() const {
		int count = 0;
		for(int ii = 0; ii < BOARD_SPACES; ii++) {
			if (((_hasPiece & BIT(ii)) != 0) &&
				(_isWhitePiece & BIT(ii)) == 0) {
				count++;
			}
		}
		return count - 2;
	}
	
	bool hasWhiteWon() const {
		if (!isFinished()) {
			return false;
		}
		int count = whitePlayerScore();
		return (count - 2 > BOARD_SPACES - count - 4);
	}
	
	bool hasPiece(int xpos, int ypos) const {
		return (_hasPiece & BITPOSITION(xpos, ypos)) != 0;
	}
	
	bool isWhitePiece(int xpos, int ypos) const {
		return (_isWhitePiece & BITPOSITION(xpos, ypos)) != 0;
	}
	
	bool isAdjacentPosition(int xpos, int ypos) const {
		return (_adjacentPositions & BITPOSITION(xpos, ypos)) != 0;
	}

	
	std::vector<Move> getPossibleMoves() const {
		std::vector<Move> result;
		std::vector<Move> noFlipResult;
		bool hasFlipMove = false;
		
		for(int jj = 0; jj < NUMBER_OF_ROWS; jj++) {
			for(int ii = 0; ii < NUMBER_OF_COLUMNS; ii++) {
				if(isAdjacentPosition(ii, jj)) {
					
					BitBoard bitsToFlip = findAllBitsToFlip(ii, jj, isWhiteTurn());
					
					if (bitsToFlip == 0) {
						if (hasFlipMove == false) {
							Move temp;
							temp.xpos = ii;
							temp.ypos = jj;
							temp.bitsToFlip = bitsToFlip;
							noFlipResult.push_back(temp);
						}
					} else {
						hasFlipMove = true;
						Move temp;
						temp.xpos = ii;
						temp.ypos = jj;
						temp.bitsToFlip = bitsToFlip;
						result.push_back(temp);
					}
				}
			}
		}
		
		if (hasFlipMove == false && !noFlipResult.empty()) {
			result.insert(result.end(), noFlipResult.begin(), noFlipResult.end());
		}
		
		return result;
	}
	
	BitBoard findBitsToFlip(int xpos, int ypos, int xdelta, int ydelta, bool isWhiteMove) const {
		int xposNew = xpos + xdelta;
		int yposNew = ypos + ydelta;
		
		BitBoard bitsToFlipTemp = 0;
		BitBoard bitsToFlipResult = 0;
		
		while(xposNew >= 0 && xposNew < NUMBER_OF_COLUMNS && yposNew >= 0 && yposNew < NUMBER_OF_ROWS &&
				hasPiece(xposNew, yposNew)) {
			
			if (isWhiteMove != isWhitePiece(xposNew, yposNew)) {
				//flipping the opposite color
				//adding this position to the bits to flip
				bitsToFlipTemp |= BITPOSITION(xposNew, yposNew);
				
			} else {
				//flipping the same color
				bitsToFlipResult = bitsToFlipTemp;
				
				bitsToFlipTemp |= BITPOSITION(xposNew, yposNew);
			}
			
			xposNew += xdelta;
			yposNew += ydelta;
		}
		
		return bitsToFlipResult;
	}
	
	BitBoard findAllBitsToFlip(int xpos, int ypos, bool isWhiteMove) const {
		BitBoard bitsToFlip = findBitsToFlip(xpos, ypos, 0, -1, isWhiteMove);
		bitsToFlip |= findBitsToFlip(xpos, ypos, 1, -1, isWhiteMove);
		bitsToFlip |= findBitsToFlip(xpos, ypos, 1, 0, isWhiteMove);
		bitsToFlip |= findBitsToFlip(xpos, ypos, 1, 1, isWhiteMove);
		bitsToFlip |= findBitsToFlip(xpos, ypos, 0, 1, isWhiteMove);
		bitsToFlip |= findBitsToFlip(xpos, ypos, -1, 1, isWhiteMove);
		bitsToFlip |= findBitsToFlip(xpos, ypos, -1, 0, isWhiteMove);
		bitsToFlip |= findBitsToFlip(xpos, ypos, -1, -1, isWhiteMove);
		return bitsToFlip;
	}
	
	
	void makeMove(Move m) {
		//we asume the move is valid and has correct bitsToFlip
		//TODO: make this method private and friends with the relevant classes
		_hasPiece |= BITPOSITION(m.xpos, m.ypos);
		_isWhitePiece ^= m.bitsToFlip;
		if (isWhiteTurn()) {
			_isWhitePiece |= BITPOSITION(m.xpos, m.ypos);
		}
		
		_adjacentPositions = (_adjacentPositions | getAdjacentPositions(m.xpos, m.ypos)) & ~_hasPiece;
		
		_turn++;
	}
	
	
	//xpos and ypos are 0 based
	bool makeMove(int xpos, int ypos) {
		
		#ifndef CODECUP
		if (!isValidMove(xpos, ypos)) {
			return false;
		}
		#endif
		
		Move moveToMake;
		moveToMake.xpos = xpos;
		moveToMake.ypos = ypos;
		moveToMake.bitsToFlip = findAllBitsToFlip(xpos, ypos, isWhiteTurn());
		makeMove(moveToMake);
		return true;
	}
	
	bool isValidMove(int xpos, int ypos) const {
		if (!isAdjacentPosition(xpos, ypos)) {
			return false;
		}
		
		auto moves = getPossibleMoves();
		
		for (uint ii = 0; ii < moves.size(); ii++) {
			if (moves[ii].xpos == xpos && moves[ii].ypos == ypos) {
				return true;
			}
		}
		return false;
	}
	
	
	friend std::ostream& operator<< (std::ostream& out, const FlippoBoard& myboard) {
		std::string board(BOARD_SPACES + NUMBER_OF_ROWS * 2, ' ');
		char c = '1';
		for (int ii = NUMBER_OF_COLUMNS; ii < (int)board.size(); ii += NUMBER_OF_COLUMNS + 2) {
			board[ii] = c;
			board[ii+1] = '\n';
			c++;
		}
		
		for(int jj = 0; jj < NUMBER_OF_ROWS; jj++) {
			for(int ii = 0; ii < NUMBER_OF_COLUMNS; ii++) {
				if (myboard.isAdjacentPosition(ii, jj)) {
					board[ii + (jj * (NUMBER_OF_COLUMNS + 2))] = '.';
				} else if (myboard.hasPiece(ii, jj)) {
					if (myboard.isWhitePiece(ii, jj)) {
						board[ii + (jj * (NUMBER_OF_COLUMNS + 2))] = 'O';
					} else {
						board[ii + (jj * (NUMBER_OF_COLUMNS + 2))] = 'X';
					}
				}
			}
		}
		
		out << "Turn " << myboard._turn << (myboard.isWhiteTurn() ? " - white (O)" : " - black (X)") << std::endl;
		out << "White: " << myboard.whitePlayerScore() << "\tBlack: " << myboard.blackPlayerScore() << std::endl;
		out << "ABCDEFGH" << std::endl;
		out << board;
		
		return out;
	}
};




class MiniMax {
	FlippoBoard _board;
	Move _move;
	int _score = 0;
	
	MiniMax(FlippoBoard board, Move currentMove, bool isWhitePlayer)
		: _board(board), _move(currentMove)
	{
		_board.makeMove(_move);
		_score = MiniMax::getScore(_board, isWhitePlayer);
	}
	
	
public:
	
	static MiniMax selectMove(int depth, FlippoBoard board, bool isWhitePlayer) {
	
		auto moves = board.getPossibleMoves();
		//std::cerr << "Possible moves " << moves.size() << std::endl;
		//std::cerr << "depth: " << depth << std::endl;
	
	
		auto best = MiniMax(board, moves[0], isWhitePlayer);
	
		for (uint ii = 1; ii < moves.size(); ii++) {
			auto current = MiniMax(board, moves[ii], isWhitePlayer);

			if (depth > 0) {
				auto temp = selectMove(depth-1, current._board, isWhitePlayer);
				current._score = temp._score;
			}
			
			if (current._score > best._score) {
				best = current;
			}
		}
		
		return best;
	}

	
	const Move getMove() const {
		return _move;
	}
	
	
private:
	static int getScore(FlippoBoard board, bool isWhitePlayer) {
		if (isWhitePlayer) {
			return board.whitePlayerScore() - board.blackPlayerScore();
		} else {
			return board.blackPlayerScore() - board.whitePlayerScore();
		}
	}
};



#define MAXDEPTH 3

class AIPlayer {
	bool _isWhitePlayer = false;
	
	//the current boardstate
	FlippoBoard _flippo;
	
	
public:
	void setAsWhitePlayer() {
		_isWhitePlayer = true;
	}
	
	bool makeMove(int xpos, int ypos) {
		return _flippo.makeMove(xpos, ypos);
	}
	
	Move selectMove() {
		auto result = MiniMax::selectMove(MAXDEPTH, _flippo, _isWhitePlayer);
		auto move = result.getMove();
		
		bool isCorrect = makeMove(move.xpos, move.ypos);
		assert(isCorrect, "selectMove gave incorrect move");
		
		return move;
	}
	
	const FlippoBoard& getBoard() const {
		return _flippo;
	}
};



//parses input and makes a move when needed.
bool parsesInput(AIPlayer* player) {
	std::string input = " ";
	bool isGoodMove = true;
	
	do {
		std::getline(std::cin, input);
		
		if (input == "Start") {
			player->setAsWhitePlayer();
			return true;
		}
		
		if (input == "Quit") {
			std::cerr << "User Quit" << std::endl;
			exit(0);
		}
		
		if (input.size() == 2) {
			char xchar = input[0];
			char ychar = input[1];
			if (xchar >= 'A' && xchar <= 'H' &&
				ychar >= '1' && ychar <= '8') {
				
				int xpos = xchar - 'A';
				int ypos = ychar - '1';
				
				isGoodMove = player->makeMove(xpos, ypos);
				if (!isGoodMove) {
					std::cerr << "Bad input" << std::endl;
				}
			} else {
				std::cerr << "Bad input" << std::endl;
			}
		} else {
			std::cerr << "Bad input" << std::endl;
		}
		
	} while (!isGoodMove);
	
	return false;
}



int main(int argc, char** argv) {
	srand(time(NULL));
	
	using std::cerr;
	using std::endl;
	
	AIPlayer player;
	
	#ifndef CODECUP
	cerr << player.getBoard();
	cerr << "Type your first move to begin or 'Start' to let the AI begin." << endl;
	#endif
	
	parsesInput(&player);
	
	while (!player.getBoard().isFinished()) {
		auto newMove = player.selectMove();
		std::cout << (char)(newMove.xpos + 'A') << (char)(newMove.ypos + '1') << endl;
		
		#ifndef CODECUP
		cerr << player.getBoard();
		#endif
		
		if (!player.getBoard().isFinished()) {
			parsesInput(&player);
		}
	}
	
	#ifndef CODECUP
	if (player.getBoard().hasWhiteWon()) {
		cerr << "white has won" << endl;
	} else {
		cerr << "black has won" << endl;
	}
	#endif
	
	return 0;
}

