#include "board.h"
#include "score.h"

enum { KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_DROP, KEY_PAUSE, KEY_QUIT };

enum { TO_BLACK_SCREEN_PAUSE = 1 };

typedef
struct tetris {
	board_t game;
	board_t next;

	score_t score;

	int stage;
	io_timer_t *timer;
	int period; // timer period;
	int paused;
	int options;
} tetris_t;


void tetris_init(tetris_t *self, int left, int top, int options);
void tetris_free(tetris_t *self);
void tetris_start(tetris_t *self);
