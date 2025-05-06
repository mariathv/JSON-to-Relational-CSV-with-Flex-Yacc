CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

# Source files
SOURCES = main.c ast.c schema.c lex.yy.c parser.tab.c
HEADERS = ast.h schema.h common.h parser.h

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Target executable
TARGET = json2relcsv

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

parser.tab.c parser.tab.h: parser.y
	bison -d $<

lex.yy.c: scanner.l parser.tab.h
	flex $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJECTS) lex.yy.c parser.tab.c parser.tab.h

.PHONY: all clean