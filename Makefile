CC=gcc
CCFLAGS=-O3 -std=gnu11 -Wall -Werror -I./src
LDFLAGS=-lm -lpulse -lpulse-simple -lttspico

SRC=$(wildcard src/**/*.c src/*.c)
OBJ=$(SRC:%.c=%.o)

TESTSRC=$(wildcard tests/**/*.c tests/*.c)
TESTOBJ=$(TESTSRC:%.c=%.o)

OBJWITHOUTMAIN := $(filter-out src/main.o,$(OBJ))

build: say-stuff

say-stuff: $(OBJ)
	$(CC) $(CCFLAGS) -o say-stuff $^ $(LDFLAGS)

# To obtain object files
%.o: %.c
	$(CC) -c $(CCFLAGS) $< -o $@

clean:
	rm -f say-stuff $(OBJ) $(TESTOBJ)
