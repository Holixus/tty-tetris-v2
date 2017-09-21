#ifndef USE_STDOUT_FOR_IO_CON

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>

#include <errno.h>

#include "io.h"
#include "io_key.h"

#include "io_con.h"

/* -------------------------------------------------------------------------- */
#define IO_CON_BUFFER_SIZE (0x1000)

/* -------------------------------------------------------------------------- */
typedef
struct conout {
	io_stream_t stream;

	char *buffer;
	unsigned int size;
	unsigned int get_ofs, put_ofs;
} conout_stream_t;


/* -------------------------------------------------------------------------- */
static conout_stream_t *io_con_stream;



/* -------------------------------------------------------------------------- */
static void io_con_event_handler(io_stream_t *stream, int events)
{
	conout_stream_t *co = (conout_stream_t *)stream;

	auto unsigned int po = co->put_ofs, go = co->get_ofs, co_size = co->size;

	auto ssize_t ret;

	if (go > po) {
		do {
			ret = write(co->stream.fd, co->buffer + go, (size_t)(co_size - go));
		} while (ret < 0 && errno == EINTR);
		if (ret < 1)
			return;

		go += (unsigned int)ret;
		if (go < co_size) {
			co->get_ofs = go;
			return;
		}
		co->get_ofs = go = 0;
	}

	if (go < po) {
		do {
			ret = write(co->stream.fd, co->buffer + go, (size_t)(po - go));
		} while (ret < 0 && errno == EINTR);
		if (ret < 1)
			return;

		go += (unsigned int)ret;
		if (go < po) {
			co->get_ofs = go;
			return;
		}
	}
	co->get_ofs = co->put_ofs = 0;
	co->stream.events = 0; // all data sent
}

/* -------------------------------------------------------------------------- */
void io_con_flush()
{
	//io_con_event_handler(&io_con_stream->stream, 0);
}

/* -------------------------------------------------------------------------- */
static void stdout_nonblock_set(int yes)
{
	int flags = fcntl(STDOUT_FILENO, F_GETFL);
	fcntl(STDOUT_FILENO, F_SETFL, yes ? flags & ~O_NONBLOCK : flags | O_NONBLOCK);
}

/* -------------------------------------------------------------------------- */
static void io_con_free_handler(io_stream_t *stream)
{
	stdout_nonblock_set(0);

	io_con_event_handler(stream, POLLOUT); // flush out buffer

	conout_stream_t *co = (conout_stream_t *)stream;
	free(co->buffer);
	co->buffer = NULL;
	co->get_ofs = co->put_ofs = co->size = 0;
}

/* -------------------------------------------------------------------------- */
static void io_con_idle_handler(io_stream_t *stream)
{
}

/* -------------------------------------------------------------------------- */
static const io_stream_ops_t io_con_ops = {
	free: io_con_free_handler,
	idle: io_con_idle_handler,
	event: io_con_event_handler
};

/* -------------------------------------------------------------------------- */
void io_con_init()
{
	conout_stream_t *co = io_con_stream = (conout_stream_t *)calloc(1, sizeof (conout_stream_t));
	io_stream_init(&io_con_stream->stream, STDOUT_FILENO, 0, &io_con_ops);

	co->buffer = (char *)malloc(co->size = IO_CON_BUFFER_SIZE);
	co->get_ofs = co->put_ofs = 0;

	stdout_nonblock_set(1);
}


/* ------------------------------------------------------------------------ */
static int io_con_write(char const *data, size_t size)
{
	if (!size)
		return 0;

	if (!io_con_stream)
		io_con_init();

	if (io_con_stream->put_ofs == io_con_stream->get_ofs) {
		ssize_t ret;

		do {
			ret = write(io_con_stream->stream.fd, data, size);
		} while (ret < 0 && errno == EINTR);

		if (ret < 0)
			return -1;

		if ((unsigned)ret == size)
			return (int)ret;

		data += (unsigned)ret;
		size -= (unsigned)ret;
	}

	unsigned int po = io_con_stream->put_ofs,
	             go = io_con_stream->get_ofs,
	             co_size = io_con_stream->size;

	ssize_t avail = (ssize_t)( po >= go ? size - po + go  : go - po );
	char *buff = io_con_stream->buffer;
	if (avail < size) {
		unsigned int new_size = io_con_stream->size;
		do {
			avail += IO_CON_BUFFER_SIZE;
			new_size += IO_CON_BUFFER_SIZE;
		} while (avail < size);

		if (go <= po)
			buff = realloc(buff, new_size);
		else {
			buff = malloc(new_size);
			memcpy(buff, io_con_stream->buffer + go, (size_t)(co_size - go));
			memcpy(buff + co_size - go, io_con_stream->buffer, (size_t)po);
			io_con_stream->put_ofs = po = co_size - (go - po);
			io_con_stream->get_ofs = go = 0;
			free(io_con_stream->buffer);
		}
		io_con_stream->buffer = buff;
		io_con_stream->size = co_size = new_size;
	}
	ssize_t tail_size = (ssize_t)(co_size - po);
	if (go <= po && tail_size < size) {
		memcpy(buff + po, data, (unsigned)tail_size);
		memcpy(buff, data + tail_size, po = (unsigned)(size - (unsigned)tail_size));
	} else {
		memcpy(buff + po, data, size);
		po += (unsigned)size;
	}
	io_con_stream->put_ofs = po;
	io_con_stream->stream.events = POLLOUT;
	return (int)size;
}

/* ------------------------------------------------------------------------ */
int io_con_put_char(char ch)
{
	char d[1] = { ch };
	return io_con_write(d, 1);
}

/* ------------------------------------------------------------------------ */
int io_con_put_str(char const *str)
{
	return io_con_write(str, strlen(str));
}

/* ------------------------------------------------------------------------ */
int io_con_vout(char const *fmt, va_list ap)
{
#ifdef USE_SNPRINTF_FOR_IO_CON
	char buf[2048];
	ssize_t len = vsnprintf(buf, sizeof(buf), fmt, ap);
	return io_con_write(buf, (size_t)len);
#else
	int wrtn = 0;
	do {
		auto char const *perc = strchr(fmt, '%');
		if (!perc) {
			wrtn += io_con_put_str(fmt);
			break;
		}
		if (fmt < perc) {
			wrtn += io_con_write(fmt, (size_t)(perc - fmt));
			fmt = perc;
		}
		++fmt;
		int width = 0;
		for (; *fmt >= '0' && *fmt <= '9'; ++fmt)
			width = width * 10 + (*fmt - '0');
		switch (*fmt) {
		case 'd': {
				char num[64], *p = num + sizeof(num);
				int arg = va_arg(ap, int),
				    minus = arg < 0;
				if (minus)
					arg = -arg;
				do {
					*--p = (char)('0' + arg % 10);
					--width;
					arg /= 10;
				} while (arg);
				if (minus)
					*--p = '-';
				for (; width > 0; --width)
					*--p = ' ';
				wrtn += io_con_write(p, (size_t)(num + (sizeof num) - p));
			} break;
		case 's': {
				char *arg = va_arg(ap, char *);
				wrtn += io_con_put_str(arg);
			} break;
		case 'c': {
				wrtn += io_con_put_char((char)va_arg(ap, int));
			} break;
		case '%':
			wrtn += io_con_put_char('%');
			break;
		default:
			wrtn += io_con_write(fmt - 1, 2);
		}
	} while (*++fmt);
	return wrtn;
#endif
}


/* ------------------------------------------------------------------------ * /
int io_con_put(char const *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int done = con_vout(fmt, ap);
	va_end(ap);
	return done;
}
*/

#endif
