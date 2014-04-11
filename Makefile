all:
	cc -std=c99 -Wall mpc.c lish.c -ledit -lm -o lish.elf
