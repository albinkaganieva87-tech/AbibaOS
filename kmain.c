#include "io.h"
#include "print.h"
#include "shell.h"
// ИСПРАВЛЕНО: Сдвигаем bg (фон), а не fg (текст)

int main() {
    fb_move_cursor(cursor_pos);

    // Печатаем копирайт и переносим курсор на строку 2
    fb_println("Copyright (C) Sus Imposter studios 2026, all rights reserved [READY]", FB_LIGHT_BLUE, FB_BLACK);
    
    // Печатаем версию ОС и переносим курсор на строку 3
    fb_println("AbibaOS version 1.0", FB_LIGHT_BLUE, FB_BLACK);
    
    // Печатаем шелл и переносим курсор на строку 4
    fb_println("SIS Abiba Shell [Version 1.0]", FB_LIGHT_BLUE, FB_BLACK);
    
    // Печатаем путь. Используем fb_print, чтобы курсор остался мигать прямо в конце этой строки!
	ShellProcess();
    while (1) {
   		 
    }
    return 0;
}
