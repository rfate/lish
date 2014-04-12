all:
	cc -std=c99 -Wall mpc.c lval.c lish.c -ledit -lm -o lish.elf

run: all
	./lish.elf