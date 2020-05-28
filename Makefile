PREFIX?=/usr/X11R6
CFLAGS?=-pedantic -Wall -O3

all:
	$(CC) $(CFLAGS) -I$(PREFIX)/include not.c -L$(PREFIX)/lib -lX11 -lpthread -o not
	cp not /usr/games

clean:
	rm -f not

