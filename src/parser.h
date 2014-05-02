#ifndef _LISH_PARSER_H
#define _LISH_PARSER_H

#include "mpc.h"

mpc_parser_t* parser_comment;
mpc_parser_t* parser_integer;
mpc_parser_t* parser_float;
mpc_parser_t* parser_boolean;
mpc_parser_t* parser_string;
mpc_parser_t* parser_symbol;
mpc_parser_t* parser_litsymbol;
mpc_parser_t* parser_operator;
mpc_parser_t* parser_tablepair;
mpc_parser_t* parser_table;
mpc_parser_t* parser_sexpr;
mpc_parser_t* parser_qexpr;
mpc_parser_t* parser_expr;
mpc_parser_t* parser_lish;

void parser_init(void);
void parser_cleanup(void);

#endif

