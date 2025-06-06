# Compilador
CC = gcc
CFLAGS = -Wall -Wextra -g

# Archivos fuente
SRC = main.c match.c dfa.c nfa.c interpreter.c
OBJ = $(SRC:.c=.o)
HEADERS = match.h dfa.h nfa.h interpreter.h

# Nombre del ejecutable
TARGET = regex_engine

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean