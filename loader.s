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

; =========================================================
; Multiboot header
; =========================================================

    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM


; =========================================================
; LOADER
; =========================================================

loader:

    ; -----------------------------------------------------
    ; Инициализируем стек ядра
    ; -----------------------------------------------------

    mov esp, kernel_stack + KERNEL_STACK_SIZE


    ; -----------------------------------------------------
    ; Очищаем VGA от вывода BIOS / предыдущего содержимого
    ; -----------------------------------------------------

    call clear_screen

    mov edi, VIDEO_MEMORY


    ; =====================================================
    ; BOOT LOG 1
    ; =====================================================

    call log_start

    mov esi, msg_1
    call print_string

    call newline


    ; =====================================================
    ; BOOT LOG 2
    ; =====================================================

    call log_start

    mov esi, msg_2
    call print_string

    call newline


    ; =====================================================
    ; BOOT LOG 3
    ; =====================================================

    call log_start

    mov esi, msg_3
    call print_string

    call newline


    ; =====================================================
    ; BOOTLOADER VERSION
    ; =====================================================

    call log_start

    mov esi, msg_bootloader
    call print_string

    call newline


    ; =====================================================
    ; KERNEL ADDRESS
    ; =====================================================

    call log_start

    mov esi, msg_kernel
    call print_string

    mov eax, 0x00100000
    call print_hex32

    call newline


    ; =====================================================
    ; KERNEL STACK
    ; =====================================================

    call log_start

    mov esi, msg_stack
    call print_string

    mov eax, kernel_stack + KERNEL_STACK_SIZE
    call print_hex32

    call newline


    ; =====================================================
    ; VGA MEMORY
    ; =====================================================

    call log_start

    mov esi, msg_vga
    call print_string

    mov eax, VIDEO_MEMORY
    call print_hex32

    call newline


    ; =====================================================
    ; PREPARE KERNEL
    ; =====================================================

    call log_start

    mov esi, msg_prepare
    call print_string

    call newline


    ; =====================================================
    ; JUMP TO KERNEL
    ; =====================================================

    call log_start

    mov esi, msg_jump
    call print_string

    call newline


    ; -----------------------------------------------------
    ; Всего bootloader занимает 9 строк:
    ;
    ; 0 - 1
    ; 1 - 2
    ; 2 - 3
    ; 3 - bootloader
    ; 4 - kernel
    ; 5 - stack
    ; 6 - VGA
    ; 7 - prepare
    ; 8 - jump
    ;
    ; C kernel начинает со строки 9.
    ; -----------------------------------------------------

    mov dword [cursor_pos], 80 * 9


    ; -----------------------------------------------------
    ; Передаём управление C kernel
    ; -----------------------------------------------------

    call main


    ; -----------------------------------------------------
    ; Если main когда-нибудь вернулся
    ; -----------------------------------------------------

    mov eax, 0xCAFEBABE


.hang:

    cli
    hlt

    jmp .hang


; =========================================================
; clear_screen
;
; Очищает весь VGA text mode экран.
;
; 80 * 25 = 2000 символов
; Каждый символ = 2 байта
; =========================================================

clear_screen:

    push eax
    push ecx
    push edi

    ; Очень важно:
    ; STOSW должен двигаться вперёд.
    cld

    mov edi, VIDEO_MEMORY

    ; AH = 0x07 -> цвет
    ; AL = 0x20 -> пробел
    ;
    ; 0x0720:
    ;
    ; 07 = light grey / black
    ; 20 = ' '

    mov ax, 0x0720

    mov ecx, 80 * 25

    rep stosw

    pop edi
    pop ecx
    pop eax

    ret


; =========================================================
; log_start
;
; Выводит:
;
; [  0.000]
; [  0.001]
; [  0.002]
;
; =========================================================

log_start:

    push eax
    push ebx
    push ecx
    push edx


    ; [
    mov al, '['
    call put_char


    ; пробел
    mov al, ' '
    call put_char


    ; Получаем boot_time
    mov eax, [boot_time]


    ; Делим на 1000
    ;
    ; EAX = секунды
    ; EDX = миллисекунды

    xor edx, edx

    mov ebx, 1000

    div ebx


    ; Печатаем секунды

    mov ecx, eax

    mov eax, ecx

    call print_3_digits


    ; .

    mov al, '.'
    call put_char


    ; Печатаем миллисекунды

    mov eax, edx

    call print_3_digits


    ; ]

    mov al, ']'
    call put_char


    ; пробел

    mov al, ' '
    call put_char


    ; Увеличиваем время

    inc dword [boot_time]


    pop edx
    pop ecx
    pop ebx
    pop eax

    ret


; =========================================================
; print_3_digits
;
; EAX = число 0..999
;
; Вывод:
;
; 000
; 001
; 042
; 999
; =========================================================

print_3_digits:

    push eax
    push ebx
    push ecx
    push edx


    ; -------------------------
    ; Сотни
    ; -------------------------

    xor edx, edx

    mov ebx, 100

    div ebx

    add al, '0'

    call put_char


    ; -------------------------
    ; Десятки
    ; -------------------------

    mov eax, edx

    xor edx, edx

    mov ebx, 10

    div ebx

    add al, '0'

    call put_char


    ; -------------------------
    ; Единицы
    ; -------------------------

    mov al, dl

    add al, '0'

    call put_char


    pop edx
    pop ecx
    pop ebx
    pop eax

    ret


; =========================================================
; print_hex32
;
; EAX = 32-bit число
;
; Например:
;
; 0x00100000
; 0x000B8000
; 0x00102000
;
; =========================================================

print_hex32:

    push eax
    push ebx
    push ecx
    push edx


    ; Сохраняем число

    mov edx, eax


    ; -------------------------
    ; "0x"
    ; -------------------------

    mov al, '0'
    call put_char

    mov al, 'x'
    call put_char


    ; -------------------------
    ; 8 hex digits
    ; -------------------------

    mov ecx, 8


.hex_loop:

    ; Переносим старший nibble
    ; в младшие 4 бита.

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


; =========================================================
; print_string
;
; ESI -> null-terminated string
;
; =========================================================

print_string:

.next:

    lodsb

    test al, al

    jz .done

    call put_char

    jmp .next


.done:

    ret


; =========================================================
; put_char
;
; AL  = ASCII character
; EDI = VGA memory position
;
; VGA cell:
;
; byte 0 = character
; byte 1 = color
;
; 0x07 = light grey on black
; =========================================================

put_char:

    mov [edi], al

    mov byte [edi + 1], 0x07

    add edi, 2

    ret


; =========================================================
; newline
;
; Переход на начало следующей строки.
; =========================================================

newline:

    push eax
    push ebx
    push edx


    ; Получаем смещение относительно VGA

    mov eax, edi

    sub eax, VIDEO_MEMORY


    ; 80 символов * 2 байта

    xor edx, edx

    mov ebx, SCREEN_WIDTH * 2

    div ebx


    ; Следующая строка

    inc eax


    ; Снова умножаем на 160

    mul ebx


    ; VIDEO_MEMORY + offset

    mov edi, VIDEO_MEMORY

    add edi, eax


    pop edx
    pop ebx
    pop eax

    ret


; =========================================================
; BOOT MESSAGES
; =========================================================

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


; =========================================================
; GNU stack protection metadata
; =========================================================

section .note.GNU-stack noalloc noexec nowrite progbits
