#include "io.h"
#include "print.h"
#include "shell.h"

int main() {
    fb_move_cursor(cursor_pos);
	
    fb_println("Copyright (C) Sus Imposter studios 2026, all rights reserved [READY]", FB_LIGHT_BLUE, FB_BLACK);
    fb_println("AbibaOS version 1.0", FB_LIGHT_BLUE, FB_BLACK);
    fb_println("SIS Abiba Shell [Version 1.0]", FB_LIGHT_BLUE, FB_BLACK);
	ShellProcess();
    while (1) {
   		 
    }
    return 0;
}
