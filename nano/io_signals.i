
/* ------------------------------------------------------------------------ */
static void sig_any(int signo)
{
	switch (signo) {
	case SIGTERM:
	case SIGINT:
		exit(-1);
	}
}

/* ------------------------------------------------------------------------ */
static void signals_init()
{
	signal(SIGTERM, sig_any);
	signal(SIGINT, sig_any);
}
