#ifndef _LISH_INTERPRETER_H
#define _LISH_INTERPRETER_H

#include "parser.h"
#include "lenv.h"

typedef struct {
  lenv_t* env;
} lish_t;

lish_t* lish_new(void);
void lish_load_file(lish_t*, char*, int);
void lish_del(lish_t*);
void lish_repl(lish_t*);

#endif
