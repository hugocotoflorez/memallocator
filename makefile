all:
	nasm -g -f elf64 mman.asm
	gcc -o test mman.o malloc.c test.c -Wall -Wextra
