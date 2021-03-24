#include <stdlib.h>

#include "nano/console.h"
#include "nano/io.h"
#include "nano/io_key.h"
#include "tetris.h"


#define LEVEL_PERIOD_FACTOR  750 // means .75
#define GAME_PERIOD         1000
#define EXPLOSION_PERIOD      60


enum {
	TS_INIT,

	TS_NEW_FIGURE,
	TS_GAMING, // -> TS_NEW_FIGURE or TS_DETONATION

	TS_DETONATION,
	TS_ANNIHILATION,
	TS_DOWNFALL, // -> NEW_FIGURE
	TS_PAUSE
};


static void tetris_refresh(tetris_t *self);

/* -------------------------------------------------------------------------- */
static void tetris_pause(tetris_t *self)
{
	io_timer_set_timeout(self->timer, (self->paused = !self->paused) ? -1 : 1000);
	if (self->paused) {
		if (self->options & TO_BLACK_SCREEN_PAUSE)
			con_cls();
	} else {
		tetris_refresh(self);
	}
//	con_flush();
}

/* -------------------------------------------------------------------------- */
static void tetris_key_handler(void *p, char const *seq, int code)
{
	auto tetris_t *self = (tetris_t *)p;
	if (self->stage != TS_GAMING)
		return;

	if (self->paused) {
		if (code == KEY_PAUSE)
			tetris_pause(self);
		return;
	}

	switch (code) {
	case KEY_LEFT:   board_figure_move(&self->game, -1, 0,  0); break;
	case KEY_RIGHT:  board_figure_move(&self->game,  1, 0,  0); break;
	case KEY_UP:     board_figure_move(&self->game,  0, 0,  1); break;
	case KEY_DOWN:   board_figure_move(&self->game,  0, 0, -1); break;
	case KEY_DROP:
		self->score.score += board_figure_drop(&self->game);
		io_timer_set_period(self->timer, 1);
		break;
	case KEY_PAUSE:  tetris_pause(self);
	}
	con_flush();
}


/* -------------------------------------------------------------------------- */
void tetris_init(tetris_t *self, int left, int top, int options)
{
	score_init(&self->score, left + 5, top + 4);
	board_init(&self->game, left + 27, top + 2, 10, 20, " .");
	board_init(&self->next, left + 10, top + 11, 4, 4, "  ");

	self->stage = TS_INIT;
	self->period = 0;
	self->timer = NULL;
	io_key_on(tetris_key_handler, self);
	self->paused = 0;
	self->options = options;
}

/* -------------------------------------------------------------------------- */
void tetris_free(tetris_t *self)
{
	board_free(&self->game);
	board_free(&self->next);
	io_timer_free(self->timer);
	io_key_off(tetris_key_handler, self);
}

/* -------------------------------------------------------------------------- */
static void tetris_tick(tetris_t *self);
static inline void tetris_timer_handler(void *self)
{
	tetris_tick((tetris_t *)self);
}

/* -------------------------------------------------------------------------- */
void tetris_start(tetris_t *self)
{
	self->timer = io_timer_alloc(tetris_timer_handler, self);
	tetris_tick(self);
}

/* -------------------------------------------------------------------------- */
static void tetris_gameover(tetris_t *self)
{
	con_xy(1, 20);
	con_put_str("Game Over!");
	exit(0);
}

/* -------------------------------------------------------------------------- */
static void tetris_refresh(tetris_t *self)
{
	static char const text[] = "\
  cursor keys\n\
       or\n\
\n\
     rotate\n\
       |\n\
      [w]\n\
<-[a] [s] [d]->\n\
\n\
    [space]\n\
       |\n\
       V\n\
\n\
  [p] - pause\n\
  [q] - quit\
";

	con_cls();
	con_box(55, 3, cyan, text);
	field_walls_put(&self->game.field);

	score_refresh(&self->score);
	board_refresh(&self->next);
	board_refresh(&self->game);
}

/* -------------------------------------------------------------------------- */
static void tetris_tick(tetris_t *self)
{
	if (self->paused)
		return;

	int next_timeout = GAME_PERIOD;
	switch (self->stage) {
	case TS_INIT:
		++self->stage;
		tetris_refresh(self);
		board_figure_new(&self->next);
		score_start(&self->score);
		next_timeout = 1;
		break;

	case TS_NEW_FIGURE:;
		int fig = self->next.figure.id;
		board_figure_new(&self->next);
		board_put(&self->next);
		score_draw(&self->score);
		int level = score_level(&self->score);
		int period = GAME_PERIOD;
		while (level-- >= 0)
			period = (period * LEVEL_PERIOD_FACTOR) / 1000;
		self->period = period;

		board_figure_set(&self->game, fig);
		if (board_figure_test(&self->game)) {
			board_put(&self->game);
			tetris_gameover(self);
			return;
		}

		board_figure_draw(&self->game, 1);
		board_put(&self->game);
		++self->stage;
		next_timeout = period;
		break;

	case TS_GAMING: // -> TS_NEW_FIGURE or TS_DETONATION
		if (!board_figure_move(&self->game, 0, 1, 0)) {
			next_timeout = self->period;
			break;
		}
		self->score.figures += 1;

		int c = board_check_detonations(&self->game);
		if (c) {
			self->score.lines += c;
			self->score.score += c * c * 10;
			score_draw(&self->score);
		}
		self->stage = c ? self->stage + 1 : TS_NEW_FIGURE;
		next_timeout = 1;
		break;

	case TS_DETONATION:
	case TS_ANNIHILATION:
		board_detonate(&self->game, self->stage == TS_DETONATION);
		++self->stage;
		next_timeout = EXPLOSION_PERIOD;
		break;

	case TS_DOWNFALL: // -> TS_NEW_FIGURE
		board_downfall(&self->game);
		self->stage = TS_NEW_FIGURE;
		next_timeout = EXPLOSION_PERIOD;
		break;
	}
	io_timer_set_period(self->timer, next_timeout);
	//con_flush();
}

