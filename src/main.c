#include "lish.h"

int main(int argc, char** argv) {
	lish_t* in = lish_new();
  lish_set_argv(in, argc, argv);

	if (argc > 1) {
		lish_load_file(in, argv[1], 0);
  } else {
  	lish_repl(in);
	}

	lish_del(in);

	return 0;
}

