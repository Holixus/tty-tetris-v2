

/* -------------------------------------------------------------------------- */
typedef
struct _key_command {
	char const seq[4];
	int code;
} key_command_t;

typedef void (seq_handler_t)(void *self, char const *seq, int code);

void io_key_set_commands(key_command_t const *cmds);
int io_key_on(seq_handler_t *onseq, void *data);
int io_key_off(seq_handler_t *onseq, void *data);
