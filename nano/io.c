#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <poll.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#include "io.h"
#include "io_timers.i"
#include "io_streams.i"
#include "io_signals.i"

#include "console.h"

#define MAX_ATEXIT_HANDLERS (10)

static void_fn_t *at_exits[MAX_ATEXIT_HANDLERS];
static size_t ae_len;
/* -------------------------------------------------------------------------- */
void io_atexit(void_fn_t *fn)
{
	at_exits[ae_len++] = fn;
}

/* -------------------------------------------------------------------------- */
void io_free()
{
	for (int i = 0; i < ae_len; ++i)
		at_exits[i]();

	con_free();
	io_timers_free();
	io_streams_free();
}

/* -------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
	ae_len = 0;
	signals_init();
	srand((unsigned int)time(NULL));
	io_timers_init();
	io_streams_init();
	con_init();

	atexit(io_free);

	start(argc, argv);
	do {
		int ret = io_streams_poll(io_get_timeout());
		if (!ret || (ret < 0 && errno != EINTR))
			return ret;
	} while (1);

	return 0;
}

