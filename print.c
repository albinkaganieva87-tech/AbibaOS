#include "print.h"
#include "io.h"


#define FB_WIDTH  80
#define FB_HEIGHT 25
#define FB_SIZE   (FB_WIDTH * FB_HEIGHT)

int cursor_pos = 0;
char *fb = (char *)0xB8000;
/* =========================================================
 * WRITE CELL
 * ========================================================= */

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


/* =========================================================
 * SCROLL SCREEN
 * ========================================================= */

static void fb_scroll(void)
{
    unsigned int row;
    unsigned int col;


    /*
     * Сдвигаем строки 1..24
     * на позиции 0..23.
     */

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


    /*
     * Очищаем последнюю строку.
     */

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


    /*
     * Курсор теперь находится
     * в начале последней строки.
     */

    cursor_pos =
        (FB_HEIGHT - 1) * FB_WIDTH;
}


/* =========================================================
 * ENSURE CURSOR IS ON SCREEN
 * ========================================================= */

static void fb_check_cursor(void)
{
    while (cursor_pos >= FB_SIZE) {
        fb_scroll();
    }
}


/* =========================================================
 * MOVE HARDWARE CURSOR
 * ========================================================= */

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


/* =========================================================
 * WRITE STRING
 * ========================================================= */

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


/* =========================================================
 * PRINT
 * ========================================================= */

void fb_print(
    char *str,
    unsigned char fg,
    unsigned char bg
)
{
    unsigned int i = 0;


    while (str[i] != '\0') {

        /*
         * Если дошли до конца экрана,
         * прокручиваем его.
         */

        fb_check_cursor();


        /*
         * Пишем символ.
         */

        fb_write_cell(
            cursor_pos * 2,
            str[i],
            fg,
            bg
        );


        /*
         * Следующая позиция.
         */

        cursor_pos++;

        i++;
    }


    /*
     * Если после печати оказались
     * за пределами экрана — scroll.
     */

    fb_check_cursor();


    /*
     * Обновляем аппаратный курсор.
     */

    fb_move_cursor(cursor_pos);
}


/* =========================================================
 * PRINTLN
 * ========================================================= */

void fb_println(
    char *str,
    unsigned char fg,
    unsigned char bg
)
{
    /*
     * Печатаем текст.
     */

    fb_print(
        str,
        fg,
        bg
    );


    /*
     * Определяем текущую строку.
     */

    unsigned int current_row =
        cursor_pos / FB_WIDTH;


    /*
     * Переходим на начало
     * следующей строки.
     */

    cursor_pos =
        (current_row + 1) * FB_WIDTH;


    /*
     * Если следующая строка уже
     * за экраном — прокручиваем.
     */

    fb_check_cursor();


    /*
     * Обновляем аппаратный курсор.
     */

    fb_move_cursor(cursor_pos);
}
