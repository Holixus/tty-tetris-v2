typedef
struct score {
	int left, top;
	int lines, figures, score, level;
	long long start_time;
	int visible;
} score_t;

void score_init(score_t *self, int x, int y);
void score_draw(score_t *self);
void score_refresh(score_t *self);

void score_start(score_t *self);
int score_level(score_t *self);
