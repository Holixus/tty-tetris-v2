/* -------------------------------------------------------------------------- */
static io_stream_t *io_streams;
static size_t io_streams_length;

/* -------------------------------------------------------------------------- */
void io_stream_init(io_stream_t *self, int fd, int events, io_stream_ops_t const *ops)
{
	self->fd = fd;
	self->events = events;
	self->ops = ops;
	self->next = io_streams;
	io_streams = self;
	io_streams_length += 1;
}

/* -------------------------------------------------------------------------- */
void io_stream_free(io_stream_t *self)
{
	for (io_stream_t **s = &io_streams; s; s = &s[0]->next)
		if (*s == self) {
			*s = self->next;
			break;
		}
	self->ops->free(self);
	close(self->fd);
	free(self);
	io_streams_length -= 1;
}

/* -------------------------------------------------------------------------- */
static inline void io_streams_init()
{
	io_streams = NULL;
}

/* -------------------------------------------------------------------------- */
static inline void io_streams_free()
{
	while (io_streams)
		io_stream_free(io_streams);
}

/* -------------------------------------------------------------------------- */
static inline int io_streams_poll(int timeout)
{
	size_t len = io_streams_length;
	struct pollfd fds[len];
	io_stream_t *streams[len];

	size_t n = 0;
	for (io_stream_t *s = io_streams; s; s = s->next) {
		if (s->ops->idle)
			s->ops->idle(s);
		if (s->events) {
			streams[n] = s;
			fds[n].fd = s->fd;
			fds[n].events = (short)s->events;
			++n;
		}
	}

	if (timeout < 0 && !n)
		return 0; // nothing to do

	int ret = poll(fds, n, timeout);
	if (ret < 0)
		return ret;

	for (size_t i = 0; i < n; ++i)
		if (fds[i].revents & streams[i]->events)
			streams[i]->ops->event(streams[i], fds[i].revents);

	return (int)n;
}
