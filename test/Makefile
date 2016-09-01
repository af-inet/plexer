TARGET  = test
CC      = gcc
CFLAGS  = -g -Wall -I.
HEADERS = test.h

OBJ = \
main.o \
../http.o

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
	rm -f *.o
	rm -f $(TARGET)-debug
	rm -f $(TARGET)
