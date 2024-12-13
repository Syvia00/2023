TARGET = vm_riskxvii
CC = gcc

CFLAGS     = -c -Wall -Wvla -Werror -O0 -std=c11
ASAN_FLAGS = -lm
SRC        = vm_riskxvii.c
OBJ        = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC) $(ASAN_FLAGS) -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(ASAN_FLAGS) $<

run:
	./$(TARGET)

test:
	echo run make run_test for result of testing correctness

run_test:
	bash bashscript.sh

clean:
	rm -f *.o *.obj $(TARGET)
