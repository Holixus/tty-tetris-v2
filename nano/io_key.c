#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include "io.h"
#include "io_key.h"

#include "io_term.i"

/* -------------------------------------------------------------------------- */
typedef struct seq_handle seq_handle_t;

struct seq_handle {
	seq_handler_t *fn;
	void *data;
};


/* -------------------------------------------------------------------------- */
#define IO_KEY_HANDLERS_LIMIT 8

/* -------------------------------------------------------------------------- */
typedef
struct keyin {
	io_stream_t stream;

	char buffer[32];
	char *data_start, *data_end;

	seq_handle_t handlers[IO_KEY_HANDLERS_LIMIT];
	int handlers_length;
} key_stream_t;




/* -------------------------------------------------------------------------- */
typedef int utf_char_t;

/* -------------------------------------------------------------------------- */
static inline utf_char_t utf8_to_glyph(char const **utf8)
{
	auto utf_char_t glyph;
	auto char const *raw = *utf8;
	if (!(*raw & 0x80))
		glyph = *raw++;
	else
		if ((*raw & 0xE0) == 0xC0) {
			if ((raw[1] & 0300) == 0200) {
				glyph = ((raw[0] & 0x1F) << 6) + (raw[1] & 0x3F);
				raw += 2;
			} else
				return 0;
		} else
			if ((*raw & 0xF0) == 0xE0) {
				if ((raw[1] & 0300) == 0200 && (raw[2] & 0300) == 0200) {
					glyph = ((raw[0] & 0xF) << 12) + ((raw[1] & 0x3F) << 6) + (raw[2] & 0x3F);
					raw += 3;
				} else
					return 0;
			} else
				if ((*raw & 0xF8) == 0xF0) {
					if ((raw[1] & 0300) == 0200 && (raw[2] & 0300) == 0200 && (raw[3] & 0300) == 0200) {
						glyph = ((raw[0] & 0x7) << 18) + ((raw[1] & 0x3F) << 12) + ((raw[2] & 0x3F) << 6) + (raw[3] & 0x3F);
						raw += 4;
					} else
						return 0;
				} else
					glyph = *raw++;
	*utf8 = raw;
	return glyph;
}

/* -------------------------------------------------------------------------- */
static int detect_char_seq(char const *text)
{
	if (!*text)
		return 0;

	char const *raw = text;
	utf_char_t glyph = utf8_to_glyph(&raw);
	if (glyph != '\033')
		return (int)(raw - text);

	char ch = *raw++;
	switch (ch) {
	case '[':
	case 'O':
		while ((*raw >= '0' && *raw <= '9') || *raw == ';')
			++raw;
		ch = *raw++;
		if (64 <= ch && ch <= 126)
			return (int)(raw - text);
		return 0;

	default:
		if (32 <= ch && ch <= 127)
			return (int)(raw + 1 - text);
	}

	return 0; /* not closed sequence */
}



static key_command_t const *io_key_commands;
static key_stream_t *io_key_stream;

/* -------------------------------------------------------------------------- */
void io_key_set_commands(key_command_t const *cmds)
{
	io_key_commands = cmds;
}

/* -------------------------------------------------------------------------- */
static void keyin_seq_handler(char const *seq)
{
	int code = -1;
	for (const key_command_t *cmd = io_key_commands; cmd->seq[0]; ++cmd)
		if (!strcmp(cmd->seq, seq)) {
			code = cmd->code;
			break;
		}
	seq_handle_t *hs = io_key_stream->handlers;
	for (int i = 0, n = io_key_stream->handlers_length; i < n; ++i)
		hs[i].fn(hs[i].data, seq, code);
}


/* -------------------------------------------------------------------------- */
static void keyin_event(io_stream_t *stream, int events)
{
	key_stream_t *ks = (key_stream_t *)stream;

	auto char *data_start = ks->data_start, *data_end = ks->data_end, *buffer = ks->buffer;

	ssize_t rn = read(stream->fd, data_end, (size_t)(sizeof ks->buffer + buffer - data_end - 1));
	if (rn < 0)
		return;

	*(data_end += rn) = 0;

	for (int s; (s = detect_char_seq(data_start)); data_start += s) {
		char seq[8];
		memcpy(seq, data_start, (unsigned)s);
		seq[s] = 0;
		keyin_seq_handler(seq);
	}

	if (data_start == buffer)
		return;

	auto long len = data_end - data_start;
	if (len > 0)
		memmove(buffer, data_start, (size_t)len);

	ks->data_start = buffer;
	ks->data_end = buffer + len;
}

/* -------------------------------------------------------------------------- */
static void keyin_free(io_stream_t *stream)
{
	term_unraw();
}

/* -------------------------------------------------------------------------- */
static const io_stream_ops_t io_key_ops = {
	free: keyin_free,
	idle: NULL,
	event: keyin_event
};

/* -------------------------------------------------------------------------- */
int io_key_on(seq_handler_t *onseq, void *data)
{
	if (!io_key_stream) {
		auto key_stream_t *ks = io_key_stream = (key_stream_t *)calloc(1, sizeof(key_stream_t));
		io_stream_init(&io_key_stream->stream, STDIN_FILENO, POLLIN, &io_key_ops);
		ks->data_start = ks->data_end = ks->buffer;
		term_raw();
	}

	int len = io_key_stream->handlers_length;
	if (len >= IO_KEY_HANDLERS_LIMIT)
		return -1;

	seq_handle_t *h = io_key_stream->handlers + len;
	h->fn = onseq;
	h->data = data;
	++io_key_stream->handlers_length;
	return 0;
}


/* -------------------------------------------------------------------------- */
int io_key_off(seq_handler_t *onseq, void *data)
{
	seq_handle_t *hs = io_key_stream->handlers;
	for (int i = 0, n = io_key_stream->handlers_length; i < n; ++i)
		if (hs[i].fn == onseq && hs[i].data == data) {
			int tail = n - i - 1;
			if (tail > 0)
				memmove(hs + i, hs + i + 1, (sizeof *hs) * (unsigned)tail);
			--io_key_stream->handlers_length;
			break;
		}

	if (!io_key_stream->handlers_length) {
		io_stream_free(&io_key_stream->stream);
		io_key_stream = NULL;
	}
	return 0;
}


/* -------------------------------------------------------------------------- * /
static void io_print_seq(char const *seq)
{
	printf("{");
	for (; *seq; ++seq) {
		char c = *seq;
		if (c < 32)
			printf("^%c", c + 64);
		else
			printf("%c", c);
	}
	printf("}");
	fflush(stdout);
}
*/
