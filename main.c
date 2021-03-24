#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#include "nano/console.h"
#include "nano/io.h"
#include "nano/io_key.h"

#include "tetris.h"

/* -------------------------------------------------------------------------- */
static const key_command_t commands[] = {
	{ "\002",      KEY_LEFT },
	{ "\006",      KEY_RIGHT },
	{ "\020",      KEY_UP },
	{ "\016",      KEY_DOWN },

/* VT100 */
	{ "\033[A",    KEY_UP },
	{ "\033[B",    KEY_DOWN },
	{ "\033[D",    KEY_LEFT },
	{ "\033[C",    KEY_RIGHT },

/* VT52 */
	{ "\033A",     KEY_UP },
	{ "\033B",     KEY_DOWN },
	{ "\033D",     KEY_LEFT },
	{ "\033C",     KEY_RIGHT },

	{ " ",         KEY_DROP },
	{ "w",         KEY_UP },
	{ "s",         KEY_DOWN },
	{ "a",         KEY_LEFT },
	{ "d",         KEY_RIGHT },
	{ "p",         KEY_PAUSE },
	{ "q",         KEY_QUIT },
	{ "", 0 }
};

/* -------------------------------------------------------------------------- */
static char *get_path_filename(char *full_path)
{
	char *slash = strrchr(full_path, '/');
	return slash ? slash + 1: full_path;
}


static tetris_t ntet;

/* -------------------------------------------------------------------------- */
static void free_all()
{
	tetris_free(&ntet);
	con_xy(1, 25);
	con_uncol();
	con_show_cursor(1);
}

/* -------------------------------------------------------------------------- */
static void main_key_handler(void *self, char const *seq, int code)
{
	if (code == KEY_QUIT)
		exit(0);
}


/* -------------------------------------------------------------------------- */
void start(int argc, char *argv[])
{
	static struct option const long_options[] = {
	/*     name, has_arg, *flag, chr */
		{ "nocolor", 0, 0, 'c' },
		{ "bsp",     0, 0, 'b' },
		{ "help",    0, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	int to_options = 0; // like black screen pause

	for (;;) {
		int option_index;
		switch (getopt_long(argc, argv, "?hcb", long_options, &option_index)) {
		case -1:
			goto _end_of_opts;

		case 'c':
			con_colors_enable(0);
			break;

		case 'b':
			to_options |= TO_BLACK_SCREEN_PAUSE;
			break;

		case 'h':
		case '?':
			con_put(
"Usage: %s <options>\n\n\
options:\n\
  -c, --nocolor\t: disable ANSI colors;\n\
  -b, --bsp\t: enable black screen pause;\n\
  -h\t\t: print this help and exit.\n\n", get_path_filename(argv[0]));
			return;
		}
	}
_end_of_opts:

	con_show_cursor(0);

	io_key_set_commands(commands);
	io_key_on(main_key_handler, NULL);

	io_atexit(free_all);

	tetris_init(&ntet, 0, 0, to_options);
	tetris_start(&ntet);
}
