global inb
global outb
global outw

section .text

inb:
    mov dx, [esp + 4]
    in al, dx
    movzx eax, al
    ret


outb:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret


outw:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret


section .note.GNU-stack noalloc noexec nowrite progbits
