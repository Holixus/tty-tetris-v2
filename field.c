#include <stdlib.h>
#include <string.h>
#include "nano/console.h"
#include "field.h"

/* ------------------------------------------------------------------------ */
void field_init(field_t *self, int left, int top, unsigned int width, unsigned int height, char const *space)
{
	auto size_t data_size = sizeof(self->data[0]) * width * height;
	self->data = (unsigned char *)malloc(data_size);
	self->back = (unsigned char *)malloc(data_size);
	memset(self->data,   0, data_size);
	memset(self->back, 255, data_size);
	self->left = left;
	self->top = top;
	self->width = width;
	self->height = height;
	self->space = space;
}

/* ------------------------------------------------------------------------ */
void field_free(field_t *self)
{
	free(self->data);
	free(self->back);
}

/* ------------------------------------------------------------------------ */
const char BLOCK[] = "[]";
const char CRACK[] = "**";
const char LEFT_WALL[] = "<!";
const char RIGHT_WALL[] = "!>";
const char LEFT_CORNER[] = "<+";
const char RIGHT_CORNER[] = "+>";
const char BOTTOM_WALL[][3] = { "--", "\\/" };

/* ------------------------------------------------------------------------ */
void field_walls_put(field_t *self)
{
	con_color(yellow);
	for (int i = 0; i < self->height; ++i) {
		con_xy(self->left - 2, self->top + i);
		con_put_str(LEFT_WALL);
		con_xy(self->left + 2 * (signed)self->width, self->top + i);
		con_put_str(RIGHT_WALL);
	}
	con_xy(self->left - 2, self->top + (signed)self->height);
	con_put_str(LEFT_CORNER);
	for (int x = 0; x < self->width; ++x)
		con_put_str(BOTTOM_WALL[0]);
	con_put_str(RIGHT_CORNER);
	con_xy(self->left, self->top + (signed)self->height + 1);
	for (int x = 0; x < self->width; ++x)
		con_put_str(BOTTOM_WALL[1]);
	con_uncol();
}

/* ------------------------------------------------------------------------ */
static void field_cell_out(field_t *self, unsigned char e)
{
	if (!e) {
		con_uncol();
		con_put_str(self->space);
	} else {
		con_bold();
		if (e < 8) {
			con_color(e);
			con_put_str(BLOCK);
		} else {
			con_color(7);
			con_put_str(CRACK);
		}
	}
}

/* ------------------------------------------------------------------------ */
void field_put(field_t *self)
{
	int pos = 0;
	for (int y = 0; y < self->height; ++y) {
		int cx = -30;
		for (int x = 0; x < self->width; ++x, ++pos) {
			if (self->data[pos] != self->back[pos]) {
				switch (x - cx) {
				default:
					con_xy(x*2 + self->left, y + self->top);
					break;
				case 3:
				case 2:
				case 1:
					do {
						field_cell_out(self, self->data[pos - x + cx]);
					} while (x != ++cx);
				case 0:;
				}
				field_cell_out(self, self->back[pos] = self->data[pos]);
				cx = x + 1;
			}
		}
	}
	con_uncol();
}

/* ------------------------------------------------------------------------ */
void field_refresh(field_t *self)
{
	memset(self->back, 255, sizeof(self->data[0]) * self->width * self->height);
	field_put(self);
}

/* ------------------------------------------------------------------------ */
void field_row_fill(field_t *self, int y, int value)
{
	if (y >= 0 && y < self->height)
		memset(self->data + self->width * (unsigned)y, value, sizeof(*self->data)*self->width);
}

/* ------------------------------------------------------------------------ */
void field_fill(field_t *self, int value)
{
	memset(self->data, value, sizeof(*self->data) * self->width * self->height);
}

/* ------------------------------------------------------------------------ */
void field_row_move(field_t *self, int dst, int src)
{
	if (dst < 0 || src < 0 || dst >= self->height || src >= self->height)
		return;

	memcpy(self->data + self->width * (unsigned)dst, self->data + self->width * (unsigned)src, sizeof(*self->data) * self->width);
	field_row_fill(self, src, 0);
}

/* ------------------------------------------------------------------------ */
int field_row_wiegh(field_t *self, int y)
{
	int c = 0;
	for (unsigned char *cell = self->data + self->width * (unsigned)y, *end = cell + self->width; cell < end; ++cell)
		c += *cell ? 1 : 0;
	return c;
}

/* ------------------------------------------------------------------------ */
int field_row_is_empty(field_t *self, int y)
{
	return !field_row_wiegh(self, y) ? 1 : 0;
}

/* ------------------------------------------------------------------------ */
int field_row_is_full(field_t *self, int y)
{
	return field_row_wiegh(self, y) == self->width ? 1 : 0;
}

/* ------------------------------------------------------------------------ */
unsigned int field_check_coords(field_t *self, int x, int y)
{
	return 0 <= x && x < self->width &&
	       0 <= y && y < self->height;
}

/* ------------------------------------------------------------------------ */
static inline unsigned int cell_offset(field_t *self, int x, int y)
{
	return (unsigned)y * self->width + (unsigned)x;
}

/* ------------------------------------------------------------------------ */
void field_set_cell(field_t *self, int x, int y, int value)
{
	if (field_check_coords(self, x, y))
		self->data[cell_offset(self, x, y)] = (cell_value_t)value;
}

/* ------------------------------------------------------------------------ */
unsigned int field_get_cell(field_t *self, int x, int y)
{
	if (field_check_coords(self, x, y))
		return self->data[cell_offset(self, x, y)];
	return (cell_value_t)~0;
}


/* ------------------------------------------------------------------------ */
int field_check_detonations(field_t *self)
{
	int c = 0;
	for (int y = 0; y < self->height; ++y)
		c += field_row_is_full(self, y)? 1 : 0;
	return c;
}


/* ------------------------------------------------------------------------ */
void field_detonate(field_t *self, int phase)
{
	for (int y = 0; y < self->height; ++y)
		if (field_row_is_full(self, y))
			field_row_fill(self, y, phase ? 8 : 0);
	field_put(self);
}


/* ------------------------------------------------------------------------ */
void field_downfall(field_t *self)
{
	for (int up = (signed)self->height - 2, down = up + 1; up >= 0; --up, --down)
		switch (field_row_is_empty(self, up) * 2 + field_row_is_empty(self, down)) {
		case 1:
			field_row_move(self, down, up); break;
		case 3:
			++down;
		}
	field_put(self);
}

