#include <unistd.h>
#include <termios.h>

static struct termios term_old;

/* -------------------------------------------------------------------------- */
static void term_unraw()
{
	tcsetattr(STDOUT_FILENO, TCSAFLUSH, &term_old);
}

/* -------------------------------------------------------------------------- */
static int term_raw()
{
	if (tcgetattr(STDOUT_FILENO, &term_old) < 0)
		return -1;

	struct termios my = term_old;
	my.c_iflag &= ~(unsigned)(BRKINT | ICRNL | INPCK | ISTRIP | IXON); //~(ICRNL | INPCK | ISTRIP | IXON);
	my.c_iflag |=  (unsigned)(IGNBRK);
	my.c_oflag &= ~(unsigned)(OPOST);
	my.c_cflag |=  (unsigned)(CS8);
	my.c_lflag &= ~(unsigned)(ECHO | ICANON | IEXTEN); // | ISIG);
	my.c_cc[VMIN] = 1;
	my.c_cc[VTIME] = 0;

	return tcsetattr(STDOUT_FILENO, TCSAFLUSH, &my);
}

