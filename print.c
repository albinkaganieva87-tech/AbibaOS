#include "print.h"
#include "io.h"


#define FB_WIDTH  80
#define FB_HEIGHT 25
#define FB_SIZE   (FB_WIDTH * FB_HEIGHT)

int cursor_pos = 0;
char *fb = (char *)0xB8000;

void fb_write_cell(
    unsigned int i,
    char c,
    unsigned char fg,
    unsigned char bg
)
{
    fb[i] = c;

    fb[i + 1] =
        ((bg & 0x0F) << 4) |
        (fg & 0x0F);
}

static void fb_scroll(void)
{
    unsigned int row;
    unsigned int col;

    for (row = 1; row < FB_HEIGHT; row++) {

        for (col = 0; col < FB_WIDTH; col++) {

            unsigned int source =
                (row * FB_WIDTH + col) * 2;

            unsigned int destination =
                ((row - 1) * FB_WIDTH + col) * 2;


            fb[destination] = fb[source];

            fb[destination + 1] =
                fb[source + 1];
        }
    }

    for (col = 0; col < FB_WIDTH; col++) {

        unsigned int index =
            ((FB_HEIGHT - 1) * FB_WIDTH + col) * 2;

        fb_write_cell(
            index,
            ' ',
            FB_LIGHT_GREY,
            FB_BLACK
        );
    }

    cursor_pos =
        (FB_HEIGHT - 1) * FB_WIDTH;
}

static void fb_check_cursor(void)
{
    while (cursor_pos >= FB_SIZE) {
        fb_scroll();
    }
}

void fb_move_cursor(unsigned short pos)
{
    /*
     * Не позволяем аппаратному курсору
     * уйти за пределы экрана.
     */

    if (pos >= FB_SIZE) {
        pos = FB_SIZE - 1;
    }


    outb(
        FB_COMMAND_PORT,
        FB_HIGH_BYTE_COMMAND
    );

    outb(
        FB_DATA_PORT,
        ((pos >> 8) & 0x00FF)
    );

    outb(
        FB_COMMAND_PORT,
        FB_LOW_BYTE_COMMAND
    );

    outb(
        FB_DATA_PORT,
        pos & 0x00FF
    );
}

void fb_write_string(
    unsigned int start_index,
    char *str,
    unsigned char fg,
    unsigned char bg
)
{
    unsigned int index = start_index;
    unsigned int i = 0;

    while (str[i] != '\0') {

        fb_write_cell(
            index,
            str[i],
            fg,
            bg
        );

        index += 2;
        i++;
    }
}

void fb_print(
    char *str,
    unsigned char fg,
    unsigned char bg
)
{
    unsigned int i = 0;


    while (str[i] != '\0') {

        fb_check_cursor();
        fb_write_cell(
            cursor_pos * 2,
            str[i],
            fg,
            bg
        );

        cursor_pos++;

        i++;
    }
    fb_check_cursor();

    fb_move_cursor(cursor_pos);
}
void fb_println(
    char *str,
    unsigned char fg,
    unsigned char bg
)
{
    fb_print(
        str,
        fg,
        bg
    );
    unsigned int current_row =
        cursor_pos / FB_WIDTH;

    cursor_pos =
        (current_row + 1) * FB_WIDTH;

    fb_check_cursor();

    fb_move_cursor(cursor_pos);
}
