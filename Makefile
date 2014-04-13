SOURCE=mpc.c builtin.c lenv.c lval.c lish.c

all:
	cc -std=c99 -Wall ${SOURCE} -ledit -lm -o lish.elf

run: all
	./lish.elf repl