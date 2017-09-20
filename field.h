
typedef
unsigned char cell_value_t;

typedef 
struct field {
	int left, top;
	unsigned int width, height;
	cell_value_t *data, *back;
	char const *space;
} field_t;


void field_init(field_t *self, int left, int top, unsigned int width, unsigned int height, char const *space);
void field_free(field_t *self);

void field_walls_put(field_t *self);
void field_put(field_t *self);
void field_refresh(field_t *self);

void field_row_fill(field_t *self, int y, int value);
void field_fill(field_t *self, int value);
void field_row_move(field_t *self, int dst, int src);
int field_row_wiegh(field_t *self, int y);
int field_row_is_empty(field_t *self, int y);
int field_row_is_full(field_t *self, int y);
unsigned int field_check_coords(field_t *self, int x, int y);

void field_set_cell(field_t *self, int x, int y, int value);
unsigned int field_get_cell(field_t *self, int x, int y);

int field_check_detonations(field_t *self);
void field_detonate(field_t *self, int phase);
void field_downfall(field_t *self);
