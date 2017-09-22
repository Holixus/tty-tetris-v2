#include <stdlib.h>
#include "board.h"


/* -------------------------------------------------------------------------- */
void board_init(board_t *self, int left, int top, unsigned int width, unsigned int height, char const *space)
{
	field_init(&self->field, left, top, width, height, space);
	figure_init(&self->figure, -1);
}


/* -------------------------------------------------------------------------- */
void board_free(board_t *self)
{
	field_free(&self->field);
	//figure_free(&self->figure);
}


/* -------------------------------------------------------------------------- */
void board_figure_set(board_t *self, int id)
{
	self->figure.id = id;
	self->figure.angle = 0;
	self->figure.x = (signed)(self->field.width - 4) >> 1;
	self->figure.y = -figure_top(id);
}

/* -------------------------------------------------------------------------- */
static int rand_fig()
{
	static int last = -1;
	int next;
	do {
		next = rand() % 7;
	} while (next == last);
	return last = next;
}


/* -------------------------------------------------------------------------- */
void board_figure_new(board_t *self)
{
	board_figure_draw(self, 0);
	board_figure_set(self, rand_fig());
	board_figure_draw(self, 1);
}

static int board_figure_submove(board_t *self, int dx, int dy, int dang)
{
	self->figure.x += dx;
	self->figure.y += dy;
	self->figure.angle += dang;
	if (figure_test(&self->figure, &self->field)) {
		if (!dx && !dy && dang && self->figure.y < 0) { // patch for "line" fig. rotation at top position
			self->figure.y = 0;
			if (!figure_test(&self->figure, &self->field))
				return 0;
		} else
			self->figure.y -= dy;
		self->figure.x -= dx;
		self->figure.angle -= dang;
		return -1; // no place to move
	}
	return 0;
}

/* -------------------------------------------------------------------------- */
int board_figure_move(board_t *self, int dx, int dy, int dang)
{
	figure_draw(&self->figure, &self->field, 0);
	int res = board_figure_submove(self, dx, dy, dang);
	figure_draw(&self->figure, &self->field, 1);
	if (res >= 0) // the figure has moved
		field_put(&self->field);
	return res;
}


/* -------------------------------------------------------------------------- */
int board_figure_drop(board_t *self)
{
	figure_draw(&self->figure, &self->field, 0);
	int c = 0;
	for (; !board_figure_submove(self, 0, 1, 0); ++c);
	figure_draw(&self->figure, &self->field, 1);
	return c;
}

