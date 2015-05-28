CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -g -O0 -D_GNU_SOURCE

getfsinfo: getfsinfo.c
	$(CC) $(CFLAGS) $< -o $@
