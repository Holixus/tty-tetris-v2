#include "figure.h"

typedef
struct board {
	field_t field;
	figure_t figure;
} board_t;

void board_init(board_t *self, int left, int top, unsigned int width, unsigned int height, char const *space);
void board_free(board_t *self);

void board_figure_set(board_t *self, int id);
void board_figure_new(board_t *self);
int board_figure_move(board_t *self, int dx, int dy, int dang);
int board_figure_drop(board_t *self);


/* -------------------------------------------------------------------------- */
static inline void board_put(board_t *self)
{
	field_put(&self->field);
}

/* -------------------------------------------------------------------------- */
static inline void board_refresh(board_t *self)
{
	field_refresh(&self->field);
}

/* -------------------------------------------------------------------------- */
static inline void board_figure_draw(board_t *self, int show)
{
	figure_draw(&self->figure, &self->field, show);
}

/* -------------------------------------------------------------------------- */
static inline int board_figure_test(board_t *self)
{
	return figure_test(&self->figure, &self->field);
}

/* -------------------------------------------------------------------------- */
static inline int board_check_detonations(board_t *self)
{
	return field_check_detonations(&self->field);
}

/* -------------------------------------------------------------------------- */
static inline void board_detonate(board_t *self, int phase)
{
	field_detonate(&self->field, phase);
}

/* -------------------------------------------------------------------------- */
static inline void board_downfall(board_t *self)
{
	field_downfall(&self->field);
}

