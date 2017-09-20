#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef USE_STDOUT_FOR_IO_CON

static inline int io_con_put_char(char ch) { putchar(ch); return 1; }
static inline int io_con_put_str(char const *str) { auto size_t s = strlen(str); fwrite(str, s, 1, stdout); return (int)s; }
static inline int io_con_vout(char const *fmt, va_list ap) { return vprintf(fmt, ap); }
static inline void io_con_init()
{ }

static inline void io_con_free()
{
	fflush(stdout);
}

static inline void io_con_flush()
{
	fflush(stdout);
}

#else

int io_con_put_char(char ch);
int io_con_put_str(char const *str);
int io_con_vout(char const *fmt, va_list ap);
void io_con_init();
static inline void io_con_free() {};
void io_con_flush();

#endif
