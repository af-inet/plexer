TARGET  = plexer
OBJ     = main.o server.o socket.o table.o http.o file.o
CC      = gcc
CFLAGS  = -g -Wall -I.
HEADERS = socket.h server.h table.h settings.h http.h file.h

.PHONY: default all clean test

default: $(TARGET)
all: $(TARGET) $(OBJ)

test: $(TARGET) $(HEADERS) $(OBJ)
	$(MAKE) all
	cd test && $(MAKE) test
	./test/test

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -o $@

debug: $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) -g -o $(TARGET)-debug

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) $(TARGET)-debug
	$(RM) $(TARGET)-debug.dSYM
	$(RM) $(TARGET)
