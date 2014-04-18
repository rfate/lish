#include "interpreter.h"

int main(int argc, char** argv) {
	interpreter_t* in = interpreter_new();

	if (argc >= 2) {
		for (int i = 1; i < argc; ++i) {
			interpreter_load_file(in, argv[i]);
		}
  } else {
  	interpreter_repl(in);
	}

	interpreter_del(in);

	return 0;
}

