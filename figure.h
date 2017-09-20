#include "field.h"

typedef
struct figure {
	int id;
	int x, y, angle;
} figure_t;

void figure_init(figure_t *self, int id);
void figure_draw(figure_t *self, field_t *field, int show);
int figure_test(figure_t *self, field_t *field);

int figure_top(int id);
