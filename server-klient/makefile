FLAGS = -Wall -Wextra -Wpedantic -std=gnu99 -g
BINS = klient server

klient: klient.c funksjoner.c klientfunksjoner.c
	gcc $(FLAGS) -o$@ $^

server: server.c funksjoner.c serverfunksjoner.c
	gcc $(FLAGS) -o$@ $^

all: $(BINS)

debug: FLAGS += -DDEBUG
debug: all

clean:
	rm $(BINS)
