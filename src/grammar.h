#ifndef _LISH_GRAMMAR_H
#define _LISH_GRAMMAR_H

#include "mpc.h"

mpc_parser_t *token_comment;
mpc_parser_t *token_nil;
mpc_parser_t *token_integer;
mpc_parser_t *token_float;
mpc_parser_t *token_boolean;
mpc_parser_t *token_string;
mpc_parser_t *token_symbol;
mpc_parser_t *token_litsymbol;
mpc_parser_t *token_operator;
mpc_parser_t *token_tablepair;
mpc_parser_t *token_table;
mpc_parser_t *token_sexpr;
mpc_parser_t *token_qexpr;
mpc_parser_t *token_expr;
mpc_parser_t *grammar_lish;

void grammar_init(void);
void grammar_cleanup(void);

#endif

