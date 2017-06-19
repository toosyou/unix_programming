#include "othello.h"

#define	PLAYER1SYM	('O')
#define	PLAYER2SYM	('X')

int board[BOARDSZ][BOARDSZ];

static int const box_top = 1;
static int const box_left = 2;
static int const boxwidth = 3;

static int use_color = 0;
static int colorborder;
static int colorplayer1;
static int colorplayer2;
static int colorcursor;
static int colormsgwarn;
static int colormsgok;
static int colorturnred;
static int colorturngreen;

bool
check_left_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dx=x-1; dx >= 0; --dx){
		if( b[y][dx] == 0)
			return false;
		else if( b[y][dx] == player ){
			if( dx != x-1){ // it's a valid place
				for(int rdx=dx+1; rdx < x; ++rdx) // flip board
					b[y][rdx] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool
check_right_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dx=x+1; dx < BOARDSZ; ++dx){
		if( b[y][dx] == 0)
			return false;
		else if( b[y][dx] == player ){
			if( dx != x+1){ // it's a valid place
				for(int rdx=dx-1; rdx > x; --rdx) // flip board
					b[y][rdx] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool
check_up_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dy=y-1; dy >= 0; --dy){
		if( b[dy][x] == 0)
			return false;
		else if( b[dy][x] == player ){
			if( dy != y-1){ // it's a valid place
				for(int rdy=dy+1; rdy < y; ++rdy) // flip board
					b[rdy][x] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool
check_down_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dy=y+1; dy < BOARDSZ; ++dy){
		if( b[dy][x] == 0)
			return false;
		else if( b[dy][x] == player ){
			if( dy != y+1){ // it's a valid place
				for(int rdy=dy-1; rdy > y; --rdy) // flip board
					b[rdy][x] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool
check_left_up_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dx=x-1, dy=y-1; dx >= 0 && dy >= 0; --dx, --dy){
		if( b[dy][dx] == 0)
			return false;
		else if( b[dy][dx] == player ){
			if( dx != x-1 ){ // it's a valid place
				for(int rdx=dx+1, rdy=dy+1; rdx < x ; ++rdx, ++rdy) // flip board
					b[rdy][rdx] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool
check_left_down_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dx=x-1, dy=y+1; dx >= 0 && dy < BOARDSZ; --dx, ++dy){
		if( b[dy][dx] == 0)
			return false;
		else if( b[dy][dx] == player ){
			if( dx != x-1 ){ // it's a valid place
				for(int rdx=dx+1, rdy=dy-1; rdx < x ; ++rdx, --rdy) // flip board
					b[rdy][rdx] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool
check_right_up_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dx=x+1, dy=y-1; dx < BOARDSZ && dy >= 0; ++dx, --dy){
		if( b[dy][dx] == 0)
			return false;
		else if( b[dy][dx] == player ){
			if( dx != x+1 ){ // it's a valid place
				for(int rdx=dx-1, rdy=dy+1; rdx > x ; --rdx, ++rdy) // flip board
					b[rdy][rdx] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool
check_right_down_board(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player){
	for(int dx=x+1, dy=y+1; dx < BOARDSZ && dy < BOARDSZ; ++dx, ++dy){
		if( b[dy][dx] == 0)
			return false;
		else if( b[dy][dx] == player ){
			if( dx != x+1 ){ // it's a valid place
				for(int rdx=dx-1, rdy=dy-1; rdx > x ; --rdx, --rdy) // flip board
					b[rdy][rdx] = player;
				return true;
			}
			return false;
		}
	}
	return false;
}

typedef bool (*check_dir_func)(int (&b)[BOARDSZ][BOARDSZ], const int x, const int y, const int player);
check_dir_func check_direction_funcs[] = {
	check_left_board,
	check_right_board,
	check_up_board,
	check_down_board,
	check_left_up_board,
	check_left_down_board,
	check_right_up_board,
	check_right_down_board
};

bool
put_piece(const int x, const int y, const bool your_turn, const int index_player, bool try_put = false){
	if(board[y][x] != 0)
		return false;
	// check around (x, y)
	bool find_pieces_around = false;
	for(int i=-1;i<=1;++i){
		for(int j=-1;j<=1;++j){
			int dx = x+i;
			int dy = y+j;
			if( dx >= 0 && dx < BOARDSZ &&
			 		dy >= 0 && dy < BOARDSZ &&
					board[dy][dx] != 0){
				find_pieces_around = true;
				break;
			}
		}
	}
	if(find_pieces_around == false)
		return false;

	// check rule
	bool is_valid_place = false;
	int new_board[BOARDSZ][BOARDSZ]; memcpy(new_board, board, sizeof(board)); // new_board = board
	int player = your_turn ? (index_player == 1 ? PLAYER1 : PLAYER2) : (index_player == 2 ? PLAYER1 : PLAYER2);
	new_board[y][x] = player;

	// check every direction
	size_t number_directions = sizeof(check_direction_funcs)/sizeof(check_dir_func);
	for(size_t i=0; i<number_directions; ++i){
		is_valid_place = (check_direction_funcs[i])(new_board, x, y, player) ? true : is_valid_place;
	}

	// update board (or not)
	if(is_valid_place){
		if(try_put == false){ // just try, don't update
			memcpy(board, new_board, sizeof(new_board));
			for(int i=0;i<BOARDSZ;++i){
				for(int j=0;j<BOARDSZ;++j){
					draw_cursor(i, j, 0);
				}
			}
			draw_cursor(x, y, 1);
		}
		return true;
	}
	return false;
}

void
init_board() {
	bzero(board, sizeof(board));
	board[3][3] = board[4][4] = PLAYER1;
	board[3][4] = board[4][3] = PLAYER2;
}

void
init_colors() {
	int coloridx = 0;	// color idx 0 is default color
	if(has_colors() == FALSE)
		return;
	start_color();
	//
	colorborder = ++coloridx;
	init_pair(colorborder, COLOR_WHITE, COLOR_BLACK);

	colorplayer1 = ++coloridx;
	init_pair(colorplayer1, COLOR_BLACK, COLOR_GREEN);

	colorplayer2 = ++coloridx;
	init_pair(colorplayer2, COLOR_BLACK, COLOR_MAGENTA);

	colorcursor = ++coloridx;
	init_pair(colorcursor, COLOR_YELLOW, COLOR_BLACK);

	colormsgwarn = ++coloridx;
	init_pair(colormsgwarn, COLOR_RED, COLOR_BLACK);

	colormsgok = ++coloridx;
	init_pair(colormsgok, COLOR_GREEN, COLOR_BLACK);

	colorturnred = ++coloridx;
	init_pair(colorturnred, COLOR_RED, COLOR_BLACK);

	colorturngreen = ++coloridx;
	init_pair(colorturngreen, COLOR_GREEN, COLOR_BLACK);
	//
	use_color = 1;
	return;
}

static chtype
BCH(int x, int y) {
	if(board[y][x] == PLAYER1) return PLAYER1SYM|COLOR_PAIR(colorplayer1);
	if(board[y][x] == PLAYER2) return PLAYER2SYM|COLOR_PAIR(colorplayer2);
	return ' ';
}

static void
draw_box(int x, int y, int ch, int color, int highlight) {
	int i;
	attron(highlight ? A_BOLD : A_NORMAL);
	attron(COLOR_PAIR(color));
	//
	move(box_top + y*2 + 0, box_left + x*(boxwidth+1));
	if(y == 0) addch(x == 0 ? ACS_ULCORNER : ACS_TTEE);
	else       addch(x == 0 ? ACS_LTEE : ACS_PLUS);
	for(i = 0; i < boxwidth; i++) addch(ACS_HLINE);
	if(y == 0) addch(x+1 == BOARDSZ ? ACS_URCORNER : ACS_TTEE);
	else       addch(x+1 == BOARDSZ ? ACS_RTEE : ACS_PLUS);
	//
	move(box_top + y*2 + 1, box_left + x*(boxwidth+1));
	addch(ACS_VLINE);
	for(i = 0; i < boxwidth/2; i++) addch(' ');
	addch(ch);
	for(i = 0; i < boxwidth/2; i++) addch(' ');
	addch(ACS_VLINE);
	//
	move(box_top + y*2 + 2, box_left + x*(boxwidth+1));
	if(y+1 == BOARDSZ) addch(x == 0 ? ACS_LLCORNER : ACS_BTEE);
	else               addch(x == 0 ? ACS_LTEE : ACS_PLUS);
	for(i = 0; i < boxwidth; i++) addch(ACS_HLINE);
	if(y+1 == BOARDSZ) addch(x+1 == BOARDSZ ? ACS_LRCORNER : ACS_BTEE);
	else               addch(x+1 == BOARDSZ ? ACS_RTEE : ACS_PLUS);
	//
	attroff(COLOR_PAIR(color));
	attroff(highlight ? A_BOLD : A_NORMAL);
}

void
draw_message(const char *msg, int highlight) {
	move(0, 0);
	attron(highlight ? A_BLINK : A_NORMAL);
	attron(COLOR_PAIR(highlight ? colormsgwarn : colormsgok));
	printw(msg);
	attroff(COLOR_PAIR(highlight ? colormsgwarn : colormsgok));
	attroff(highlight ? A_BLINK : A_NORMAL);
	return;
}

void
draw_cursor(int x, int y, int show) {
	draw_box(x, y, BCH(x, y), show ? colorcursor : colorborder, show);
	return;
}

void
draw_turn(bool your_turn, int index_player){
	char msg[500]; sprintf(msg, "Player #%d: %s\n", index_player, your_turn ? "It's my turn" : "Waiting for peer");
	move(0, 0);
	attron( COLOR_PAIR(your_turn ? colorturngreen : colorturnred) );
	printw(msg);
	attroff( COLOR_PAIR(your_turn ? colorturngreen : colorturnred) );
	return;
}

void
draw_board() {
	int i, j;
	for(i = 0; i < BOARDSZ; i++) {
		for(j = 0; j < BOARDSZ; j++) {
			draw_box(i, j, BCH(i, j), colorborder, 0);
		}
	}
	return;
}

void
draw_score() {
	int i, j;
	int black = 0, white = 0;
	for(i = 0; i < BOARDSZ; i++) {
		for(j = 0; j < BOARDSZ; j++) {
			if(board[i][j] == PLAYER1) white++;
			if(board[i][j] == PLAYER2) black++;
		}
	}
	attron(A_BOLD);
	move(box_top+3, box_left + 4*BOARDSZ + 10);
	printw("Player #1 ");
	addch(PLAYER1SYM|COLOR_PAIR(colorplayer1));
	printw(" : %d", white);
	move(box_top+5, box_left + 4*BOARDSZ + 10);
	printw("Player #2 ");
	addch(PLAYER2SYM|COLOR_PAIR(colorplayer2));
	printw(" : %d", black);
	attroff(A_BOLD);
	return;
}
