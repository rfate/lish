#include "lish.h"

int main(int argc, char** argv) {
	lish_t* in = lish_new();

	if (argc >= 2) {
		for (int i = 1; i < argc; ++i) {
			lish_load_file(in, argv[i], 0);
		}
  } else {
  	lish_repl(in);
	}

	lish_del(in);

	return 0;
}

