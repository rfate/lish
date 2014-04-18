#ifndef _LISH_INTERPRETER_H
#define _LISH_INTERPRETER_H

#include "parser.h"
#include "lenv.h"

typedef struct {
  lenv_t* env;
} interpreter_t;

interpreter_t* interpreter_new(void);
void interpreter_load_file(interpreter_t*, char*, int);
void interpreter_del(interpreter_t*);
void interpreter_repl(interpreter_t*);

#endif
