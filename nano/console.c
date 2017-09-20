#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define USE_IO_CON

#include "console.h"

#ifndef USE_IO_CON

#define CONSOLE_BUFFER_SIZE 0x1000

/* ------------------------------------------------------------------------- */
static void safe_write(int fd, char const *data, size_t size)
{
	if (!size)
		return;

	ssize_t ret;
_next_part:
	do {
		ret = write(fd, data, size);
	} while (ret < 0 && errno == EINTR);

	if (ret < 0)
		exit(1);

	if (ret < size) {
		data += (unsigned)ret;
		size -= (unsigned)ret;
		goto _next_part;
	}
}


/* ------------------------------------------------------------------------ */
static
struct _con_buf_ {
	char *start;
	char *end;
	char *ptr;
} con_buf;

static inline void con_buf_init()
{
	con_buf.end = (con_buf.start = con_buf.ptr = (char *)malloc(CONSOLE_BUFFER_SIZE)) + CONSOLE_BUFFER_SIZE;
	con_buf.start[0] = 0;
}

static inline void con_buf_free()
{
	free(con_buf.start);
	con_buf.start = con_buf.end = con_buf.ptr = NULL;
}

/* ------------------------------------------------------------------------ */
void con_flush()
{
	safe_write(STDOUT_FILENO, con_buf.start, (size_t)(con_buf.ptr - con_buf.start));
	con_buf.ptr = con_buf.start;
}

/* ------------------------------------------------------------------------ */
static void con_buf_extend(size_t req_size)
{
	size_t tail = (unsigned)(con_buf.end - con_buf.ptr);

	if (req_size < tail - 1)
		return;

	size_t buf_size = (unsigned)(con_buf.end - con_buf.start),
	       length = (unsigned)(con_buf.ptr - con_buf.start),
	       new_size = buf_size;
	do {
		new_size *= 2;
	} while (new_size - length > req_size);

	char *heap = realloc(con_buf.start, new_size);

	con_buf.ptr = (con_buf.ptr - con_buf.start) + heap;
	con_buf.start = heap;
	con_buf.end = heap + new_size;
}

/* ------------------------------------------------------------------------ */
int con_put_char(char ch)
{
	con_buf_extend(2);
	*con_buf.ptr++ = ch;
	return 1;
}

/* ------------------------------------------------------------------------ */
int con_put_str(char const *str)
{
	size_t len = strlen(str);
	con_buf_extend(len + 1);
	memcpy(con_buf.ptr, str, len + 1);
	con_buf.ptr += len;
	return (signed)len;
}

/* ------------------------------------------------------------------------ */
int con_vout(char const *fmt, va_list ap)
{
	int done;
	do {
		size_t limit = (unsigned)(con_buf.end - con_buf.ptr);
		done = vsnprintf(con_buf.ptr, limit, fmt, ap);

		if (done < limit && done >= 0)
			break;

		con_buf_extend((unsigned)done+1);
	} while (1);

	con_buf.ptr += done;
	return done;
}

#endif

/* ------------------------------------------------------------------------ */
int con_put(char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int done = io_con_vout(fmt, ap);
	va_end(ap);
	return done;
}

static int _x, _y, _colored;
static int _color, _bgr, _bold;

/* ------------------------------------------------------------------------ */
void con_init()
{
	_x = _y = -1;
	_colored = 1;
	_color = white;
	_bgr = _bold = 0;

	//con_buf_init();
	io_con_init();
}

/* ------------------------------------------------------------------------ */
void con_colors_enable(int yes)
{
	_colored = yes ? 1 : 0;
}

/* ------------------------------------------------------------------------ */
void con_free()
{
	//con_buf_free();
	con_uncol();
	io_con_free();
}


int con_xy(int x, int y)
{
	return con_put("\x1b[%d;%dH", _y = y, _x = x);
}

int con_lf()
{
	return con_xy(_x, _y + 1);
}

int con_cls()
{
	return con_put_str("\x1b[2J");
}

int con_color(int c)
{
	return c == _color || !_colored ? 0 : con_put("\x1b[3%dm", _color = c);
}

int con_bold()
{
	return _bold || !_colored ? 0 : (_bold = 1, con_put_str("\x1b[1m"));
}

int con_uncol()
{
	if (_colored && (_color != 7 || _bgr || _bold)) {
		_color = 7; _bgr = 0; _bold = 0;
		return con_put_str("\x1b[0m");
	}
	return 0;
}


int con_bgr(int c)
{
	return c == _bgr || !_colored ? 0 : con_put("\x1b[4%dm", _bgr = c);
}

int con_unbold()
{
	if (!_colored || !_bold) return 0;
	int col = _color, bg = _bgr;
	return con_uncol() + con_color(col) + con_bgr(bg);
}


int con_show_cursor(int yes)
{
	return con_put("\x1b[?25%c", yes ? 'h' : 'l');
}

/*int con_box(int x, int y, char const *str)
{
	con_xy(x, y);
	while (*str) {
		str += con_put_str(str) + 1;
		con_lf();
	}
	return 0;
}*/

void con_box(int x, int y, int color, char const *text)
{
	con_xy(x, y);
	con_color(color);

	for (char const *i = text; *i; ++i)
		switch (*i) {
		case '\n':
			con_xy(x, ++y);
			break;
		case '[':
			con_bold();
			con_put_char(*i);
			break;
		case ']':
			con_put_char(*i);
			con_unbold();
			break;
		default:
			con_put_char(*i);
		}
	con_uncol();
}
