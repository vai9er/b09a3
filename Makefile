CC = gcc
DEPS = helpers.h parseArgs.h stats_functions.h

mySystemStats: main.c $(DEPS)
	$(CC) -o $@ $<

clean:
	rm -f mySystemStats