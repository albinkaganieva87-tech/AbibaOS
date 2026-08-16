global loader

MAGIC_NUMBER equ 0x1BADB002
FLAGS equ 0x0
CHECKSUM equ -MAGIC_NUMBER

KERNEL_STACK_SIZE equ 4096

VIDEO_MEMORY equ 0xB8000
SCREEN_WIDTH equ 80


extern main
extern cursor_pos


section .bss

align 4

kernel_stack:
    resb KERNEL_STACK_SIZE

boot_time:
    dd 0


section .text

align 4

    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM


loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    call clear_screen

    mov edi, VIDEO_MEMORY
    call log_start

    mov esi, msg_1
    call print_string

    call newline
    call log_start

    mov esi, msg_2
    call print_string

    call newline
    call log_start

    mov esi, msg_3
    call print_string

    call newline

    call log_start

    mov esi, msg_bootloader
    call print_string

    call newline
    
    call log_start

    mov esi, msg_kernel
    call print_string

    mov eax, 0x00100000
    call print_hex32

    call newline

    call log_start

    mov esi, msg_stack
    call print_string

    mov eax, kernel_stack + KERNEL_STACK_SIZE
    call print_hex32

    call newline
    
    call log_start

    mov esi, msg_vga
    call print_string

    mov eax, VIDEO_MEMORY
    call print_hex32

    call newline

    call log_start

    mov esi, msg_prepare
    call print_string

    call newline

    call log_start

    mov esi, msg_jump
    call print_string

    call newline

    mov dword [cursor_pos], 80 * 9

    call main

    mov eax, 0xCAFEBABE


.hang:

    cli
    hlt

    jmp .hang

clear_screen:

    push eax
    push ecx
    push edi

    ; Очень важно:
    ; STOSW должен двигаться вперёд.
    cld

    mov edi, VIDEO_MEMORY

    mov ax, 0x0720

    mov ecx, 80 * 25

    rep stosw

    pop edi
    pop ecx
    pop eax

    ret

log_start:

    push eax
    push ebx
    push ecx
    push edx


    ; [
    mov al, '['
    call put_char

    mov al, ' '
    call put_char

    mov eax, [boot_time]

    xor edx, edx

    mov ebx, 1000

    div ebx


    mov ecx, eax

    mov eax, ecx

    call print_3_digits


    mov al, '.'
    call put_char

    mov eax, edx

    call print_3_digits

    mov al, ']'
    call put_char

    mov al, ' '
    call put_char

    inc dword [boot_time]


    pop edx
    pop ecx
    pop ebx
    pop eax

    ret

print_3_digits:

    push eax
    push ebx
    push ecx
    push edx

    xor edx, edx

    mov ebx, 100

    div ebx

    add al, '0'

    call put_char

    mov eax, edx

    xor edx, edx

    mov ebx, 10

    div ebx

    add al, '0'

    call put_char

    mov al, dl

    add al, '0'

    call put_char


    pop edx
    pop ecx
    pop ebx
    pop eax

    ret
    
print_hex32:

    push eax
    push ebx
    push ecx
    push edx


    mov edx, eax

    mov al, '0'
    call put_char

    mov al, 'x'
    call put_char

    mov ecx, 8


.hex_loop:

    rol edx, 4

    mov eax, edx

    and eax, 0x0F


    ; 0..9

    cmp al, 9

    jbe .number


    ; A..F

    add al, 'A' - 10

    jmp .print


.number:

    add al, '0'


.print:

    call put_char

    loop .hex_loop


    pop edx
    pop ecx
    pop ebx
    pop eax

    ret

print_string:

.next:

    lodsb

    test al, al

    jz .done

    call put_char

    jmp .next


.done:

    ret

put_char:

    mov [edi], al

    mov byte [edi + 1], 0x07

    add edi, 2

    ret

newline:

    push eax
    push ebx
    push edx

    mov eax, edi

    sub eax, VIDEO_MEMORY

    xor edx, edx

    mov ebx, SCREEN_WIDTH * 2

    div ebx

    inc eax

    mul ebx

    mov edi, VIDEO_MEMORY

    add edi, eax


    pop edx
    pop ebx
    pop eax

    ret
    
msg_1:
    db "1", 0


msg_2:
    db "2", 0


msg_3:
    db "3", 0


msg_bootloader:
    db "Abiba OS bootloader v1.0", 0


msg_kernel:
    db "Loading kernel at ", 0


msg_stack:
    db "Setting kernel stack at ", 0


msg_vga:
    db "Initializing VGA memory at ", 0


msg_prepare:
    db "Preparing kernel execution", 0


msg_jump:
    db "Jumping to kernel entry point", 0

section .note.GNU-stack noalloc noexec nowrite progbits
