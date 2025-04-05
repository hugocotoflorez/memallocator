SRC = mman.o malloc.c
OPTS = -Wall -Wextra -std=gnu99

all:
	nasm -g -f elf64 mman.asm
	gcc -o test $(SRC) test.c $(OPTS)
	gcc -o autotest $(SRC) autotest.c $(OPTS)
	./autotest


