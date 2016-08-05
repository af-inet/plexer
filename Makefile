TARGET  = plexer
OBJ     = main.o server.o socket.o table.o http.o
CC      = gcc
CFLAGS  = -g -Wall -I.
HEADERS = socket.h server.h table.h settings.h http.h

.PHONY: default all clean

default: $(TARGET)
all: default

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
