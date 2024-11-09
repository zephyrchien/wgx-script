CC = clang
CFLAGS = -Wall -O3 -march=native -ffast-math
LDFLAGS = -lncurses

all: clean wgx-tui wgx-cmd

wgx-tui: tui.o wgx.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ && strip $@

wgx-cmd: tui.o wgx.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ && strip $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o wgx-tui wgx-cmd

.PHONY: all clean
