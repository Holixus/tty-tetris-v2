
typedef struct io_timer io_timer_t;
typedef void (timer_handler_t)(void *custom);

struct io_timer {
	io_timer_t *next;
	long long next_time;
	int period;
	timer_handler_t *handler;
	void *custom;
};


long long io_now();

io_timer_t *io_timer_alloc(timer_handler_t *handler, void *custom);

void io_timer_free(io_timer_t *self);
void io_timer_set_timeout(io_timer_t *self, int timeout);
void io_timer_set_period(io_timer_t *self, int period);
void io_timer_stop(io_timer_t *self);


/* -------------------------------------------------------------------------- */
typedef struct stream io_stream_t;

typedef
struct {
	void (*free)(io_stream_t *stream);
	void (*idle)(io_stream_t *stream);
	void (*event)(io_stream_t *stream, int events);
} io_stream_ops_t;

struct stream {
	io_stream_t *next;
	int fd;
	int events;
	io_stream_ops_t const *ops;
};


void io_stream_init(io_stream_t *self, int fd, int events, io_stream_ops_t const *ops);
void io_stream_free(io_stream_t *self);

/* -------------------------------------------------------------------------- */

void start(int argc, char *argv[]);

int io_init();
void io_free();
int io_loop();

typedef
void (void_fn_t)();

void io_atexit(void_fn_t *fn);
