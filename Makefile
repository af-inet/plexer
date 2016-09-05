TARGET  = plexer
CC      = gcc
CFLAGS  = -g -Wall -I.
OBJ     = main.o core.o socket.o table.o http.o file.o connection.o pool.o
HEADERS =        core.h socket.h table.h http.h file.h connection.h pool.h settings.h

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
