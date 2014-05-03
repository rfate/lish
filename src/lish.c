#include "linenoise.h"
#include "mpc.h"
#include "lish.h"
#include "grammar.h"

lish_t* lish_new(void) {
  grammar_init();

  lish_t* in = malloc(sizeof(lish_t));
  in->env = lenv_new();
  lenv_add_builtins(in->env);

  lish_load_file(in, "lib/core.lish", !LISH_LOCAL_LIBS);

  return in;
}

void lish_set_argv(lish_t* in, int argc, char** argv) {
  lval_t* argvKey   = lval_sym("ARGV", 0);
  lval_t* argvConst = lval_qexpr();

  for (int i = 1; i < argc; ++i) {
    argvConst = lval_add(argvConst, lval_str(argv[i]));
  }

  lenv_def(in->env, argvKey, argvConst);
  lval_del(argvKey);
  lval_del(argvConst);
}

void lish_load_file(lish_t* in, char* filename, int usePath) {
  lval_t* libArgs = lval_sexpr();
  libArgs = lval_add(libArgs, lval_str(filename));
  libArgs = lval_add(libArgs, lval_bool(usePath));

  lval_t* lib = builtin_load(in->env, libArgs);
  if (lib->type == LVAL_ERR) { lval_println(lib); }
  lval_del(lib);
}

void lish_del(lish_t* in) {
  lenv_del(in->env);
  free(in);

  grammar_cleanup();
}

void lish_repl(lish_t* in) {
  puts("Lish " LISH_VERSION);

  while (1) {
    mpc_result_t r;

    char* input = linenoise("lish> ");

    if (!input)
      break;

    linenoiseHistoryAdd(input);

    if (mpc_parse("<stdin>", input, grammar_lish, &r)) {
      lval_t* x = lval_eval(in->env, lval_read(r.output));
      lval_println(x);
      lval_del(x);

      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }
}
