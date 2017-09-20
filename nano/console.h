
#include <stdarg.h>

#include "io_con.h"

enum { black, red, green, yellow, blue, fuchsia, cyan, white };

void con_init();
void con_free();

void con_colors_enable(int yes);

static inline int con_put_char(char ch) { return io_con_put_char(ch); }
static inline int con_put_str(char const *str) { return io_con_put_str(str); }

static inline int con_vout(char const *fmt, va_list ap) { return io_con_vout(fmt, ap); }
int con_put(char const *fmt, ...) __attribute__ ((format (printf, 1, 2)));


int con_xy(int x, int y);
int con_lf();
int con_cls();
int con_color(int c);
int con_bold();
int con_uncol();
int con_bgr(int c);
int con_unbold();
int con_show_cursor(int yes);
void con_box(int x, int y, int color, char const *str);

static inline void con_flush() { io_con_flush(); }
