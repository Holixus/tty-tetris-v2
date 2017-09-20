#include "nano/console.h"
#include "nano/io.h"
#include "score.h"



#define LEVEL_UP              20 // 

/* ------------------------------------------------------------------------ */
void score_init(score_t *self, int x, int y)
{
	self->left = x;
	self->top = y;
	self->visible = 
	self->score = 
	self->lines = 
	self->figures = 
	self->level = 0;
	self->start_time = io_now();
}

/* ------------------------------------------------------------------------ */
void score_start(score_t *self)
{
	self->score = 
	self->lines = 
	self->figures = 
	self->level = 0;
	self->start_time = io_now();
}

/* ------------------------------------------------------------------------ */
int score_level(score_t *self)
{
	return self->level = self->figures / LEVEL_UP;
}

/* ------------------------------------------------------------------------ */
void score_draw(score_t *self)
{
	con_color(green);
	if (!self->visible) {
		con_box(self->left, self->top, green, "\
Lines:\n\
Figures:\n\
Level:\n\
Score:\n");
		self->visible = 1;
	}

	con_xy(self->left + 10, self->top);
	con_bold();
	con_put("%4d", self->lines); con_lf();
	con_put("%4d", self->figures); con_lf();
	con_put("%4d", self->level + 1); con_lf();
	con_put("%4d", self->score);
	con_uncol();
}

/* ------------------------------------------------------------------------ */
void score_refresh(score_t *self)
{
	self->visible = 0;
	score_draw(self);
}
