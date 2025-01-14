; mmap and munmap syscall for x86-64
section .data

section .text

mmap:
        mov rax, 9
        syscall
        ret

munmap:
        mov rax, 11
        syscall
        ret

