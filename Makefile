TARGET  = plexer
CC      = gcc
CFLAGS  = -g -Wall -I. -Wno-initializer-overrides
OBJ     = main.o socket.o http.o file.o server.o connection.o error.c
HEADERS =        socket.h http.h file.h server.h connection.h error.h

# where plexer will be installed
INSTALL_DEST = /usr/local/bin

.PHONY: default all clean test

default: all
all: $(TARGET) $(OBJ) test

install: $(TARGET)
	chmod 755 ./$(TARGET)
	cp ./$(TARGET) $(INSTALL_DEST)/$(TARGET)

uninstall:
	rm $(INSTALL_DEST)/$(TARGET)

test: $(HEADERS) $(OBJ)
	$(MAKE) $(OBJ)
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
