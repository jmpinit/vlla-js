PROG := vlla-js

ifeq ($(OS),Windows_NT)
S := \\
else
S := /
endif

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES = vlla-js.c \
	  duktape.c

INCLUDES = -Isrc$(S)inc -I/usr/local/include
OBJECTS = $(patsubst %,$(OBJDIR)$(S)%,$(SOURCES:.c=.o))

CFLAGS := -Wall -pedantic -std=gnu99 -g -Os -gstabs -ggdb -fstrict-aliasing
LFLAGS = -lm -L/usr/local/lib
CC := gcc

all: $(PROG)

run: $(PROG)
	bin$(S)$(PROG)
	
debug: $(PROG)
	gdb bin$(S)$(PROG)

# linking the program.
$(PROG): $(OBJECTS) $(BINDIR)
	$(CC) $(OBJECTS) -o $(BINDIR)$(S)$(PROG) $(LFLAGS)

$(BINDIR):
	mkdir -p $(BINDIR)

# compiling source files.
$(OBJDIR)$(S)%.o: $(SRCDIR)$(S)%.c $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -s -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
ifeq ($(OS),Windows_NT)
	del $(OBJECTS)
else
	rm $(OBJECTS)
endif

.PHONY: all clean

