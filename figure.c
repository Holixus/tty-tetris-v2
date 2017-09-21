#include "figure.h"

#define FIGURE_LENGTH 4

/*
	0 1 2 3      0 1         1
	4 5 6 7        5 6     4 5
	8 9 A B                8
	C D E F
*/

/* ------------------------------------------------------------------------ */
static unsigned int get_figure(int fig, int angle)
{
	static const unsigned short c_figures[] = {
		0x0156, 0x1458, 0x0156, 0x1458, // z
		0x1245, 0x0459, 0x1245, 0x0459, // s
		0x2456, 0x159A, 0x0124, 0x0159, // L
		0x0126, 0x1589, 0x0456, 0x1259, // j
		0x1459, 0x1456, 0x1569, 0x0125, // T
		0x4567, 0x159d, 0x4567, 0x159d, // |
		0x1256, 0x1256, 0x1256, 0x1256  // o
	};

	//assert(fig >= 0 && fig < 7);
	return c_figures[fig * 4 + (angle & 3)];
}

/* ------------------------------------------------------------------------ */
void figure_init(figure_t *self, int id)
{
	self->x = self->y = self->angle = 0;
	self->id = id;
}

/* ------------------------------------------------------------------------ */
int figure_top(int id)
{
	unsigned int fig = get_figure(id, 0);
	unsigned int top = 10;
	for (int i = 0; i < FIGURE_LENGTH && top; ++i, fig >>= 4) {
		unsigned int y = ((fig >> 2) & 3);
		if (y < top)
			top = y;
	}
	return (signed)top;
}

/* ------------------------------------------------------------------------ */
void figure_draw(figure_t *self, field_t *field, int show)
{
	int id = self->id;
	if (id < 0)
		return;
	unsigned char fill = (unsigned char)(id + 1);
	if (!show)
		fill = 0;
	unsigned int fig = get_figure(id, self->angle);
	int x = self->x, y = self->y;
	for (int i = 0; i < FIGURE_LENGTH; ++i, fig >>= 4)
		field_set_cell(field, x + (signed)(fig & 3), y + (signed)((fig >> 2) & 3), fill);
}


/* ------------------------------------------------------------------------ */
int figure_test(figure_t *self, field_t *field)
{
	int id = self->id;
	if (id < 0)
		return 1;
	unsigned int fig = get_figure(id, self->angle);
	int x = self->x, y = self->y;
	for (int i = 0; i < FIGURE_LENGTH; ++i, fig >>= 4)
		if (field_get_cell(field, x + (signed)(fig & 3), y + (signed)((fig >> 2) & 3)))
			return 1;
	return 0;
}

