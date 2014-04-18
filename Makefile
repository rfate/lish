SOURCE=mpc.c builtin.c lenv.c lval.c lish.c

all:
	cc -std=c11 -Wall ${SOURCE} -ledit -lm -o lish.elf

run: all
	./lish.elf

dbg: all
	gdb -ex run ./lish.elf
