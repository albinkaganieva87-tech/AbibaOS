#include "shell.h"
#include "print.h"
#include "io.h"


#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64

#define HISTORY_SIZE 10

#define KEY_UP    0x11
#define KEY_DOWN  0x12


char *path = "home/user/shell>";

static char history[HISTORY_SIZE][MAX_INPUT_LENGTH];

static unsigned int history_count = 0;

static unsigned int history_position = 0;

/*
 * Здесь сохраняем строку, которую пользователь набрал
 * перед первым нажатием стрелки вверх.
 */
static char history_temp[MAX_INPUT_LENGTH];

static const char keyboard_map[128] = {
    0,
    27,

    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',

    '-',
    '=',

    '\b',
    '\t',

    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',

    '[',
    ']',

    '\n',

    0,

    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',

    ';',
    '\'',
    '`',

    0,

    '\\',

    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',

    ',',
    '.',
    '/',

    0,

    '*',

    0,

    ' '
};

static void strcpy_local(char *destination, const char *source)
{
    unsigned int i = 0;

    while (source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }

    destination[i] = '\0';
}

static int strcmp_local(const char *a, const char *b)
{
    unsigned int i = 0;

    while (a[i] != '\0' && b[i] != '\0') {

        if (a[i] != b[i]) {
            return 1;
        }

        i++;
    }

    if (a[i] != b[i]) {
        return 1;
    }

    return 0;
}

static unsigned char keyboard_read_scancode(void)
{
    unsigned char status;

    while (1) {

        status = inb(KEYBOARD_STATUS_PORT);

        /*
         * Bit 0:
         *
         * 1 = данные готовы
         * 0 = ждём
         */

        if (status & 1) {
            return inb(KEYBOARD_DATA_PORT);
        }
    }
}


char getc(void)
{
    unsigned char scancode;
    unsigned char extended;

    while (1) {

        scancode = keyboard_read_scancode();

        if (scancode == 0xE0) {

            extended = keyboard_read_scancode();

            if (extended == 0x48) {
                return KEY_UP;
            }


            if (extended == 0x50) {
                return KEY_DOWN;
            }

            continue;
        }

        if (scancode & 0x80) {
            continue;
        }


        if (scancode >= 128) {
            continue;
        }

        if (keyboard_map[scancode] != 0) {
            return keyboard_map[scancode];
        }
    }
}

static void clear_input(char *buffer)
{
    unsigned int length = 0;

    while (buffer[length] != '\0') {
        length++;
    }


    while (length > 0) {

        length--;

        cursor_pos--;

        fb_write_cell(
            cursor_pos * 2,
            ' ',
            FB_LIGHT_BLUE,
            FB_BLACK
        );
    }

    buffer[0] = '\0';

    fb_move_cursor(cursor_pos);
}

static void history_add(const char *command)
{
    unsigned int i;

    if (command[0] == '\0') {
        return;
    }

    if (history_count >= HISTORY_SIZE) {

        for (i = 1; i < HISTORY_SIZE; i++) {
            strcpy_local(
                history[i - 1],
                history[i]
            );
        }

        history_count = HISTORY_SIZE - 1;
    }

    strcpy_local(
        history[history_count],
        command
    );

    history_count++;

    /*
     * Начинаем навигацию с позиции
     * после последней команды.
     */

    history_position = history_count;
}

static void history_load(
    char *buffer,
    unsigned int position
)
{
    clear_input(buffer);

    strcpy_local(
        buffer,
        history[position]
    );

    fb_print(
        buffer,
        FB_WHITE,
        FB_BLACK
    );
}

void gets(char *buffer)
{
    unsigned int index = 0;

    char c;

    buffer[0] = '\0';

    history_position = history_count;

    history_temp[0] = '\0';

    while (1) {

        c = getc();

        if (c == '\n') {

            buffer[index] = '\0';

            history_add(buffer);

            fb_println(
                "",
                FB_WHITE,
                FB_BLACK
            );

            return;
        }


        if (c == '\b') {

            if (index > 0) {

                index--;

                buffer[index] = '\0';

                cursor_pos--;

                fb_write_cell(
                    cursor_pos * 2,
                    ' ',
                    FB_LIGHT_BLUE,
                    FB_BLACK
                );


                fb_move_cursor(cursor_pos);
            }

            continue;
        }

        if (c == KEY_UP) {

            if (history_count == 0) {
                continue;
            }

            if (history_position == history_count) {

                strcpy_local(
                    history_temp,
                    buffer
                );
            }

            if (history_position > 0) {

                history_position--;

                history_load(
                    buffer,
                    history_position
                );

                /*
                 * Обновляем index.
                 */

                index = 0;

                while (buffer[index] != '\0') {
                    index++;
                }
            }

            continue;
        }

        if (c == KEY_DOWN) {

            if (history_position >= history_count) {
                continue;
            }


            history_position++;

            if (history_position == history_count) {

                clear_input(buffer);

                strcpy_local(
                    buffer,
                    history_temp
                );

                fb_print(
                    buffer,
                    FB_WHITE,
                    FB_BLACK
                );
            }

            else {

                history_load(
                    buffer,
                    history_position
                );
            }

            index = 0;

            while (buffer[index] != '\0') {
                index++;
            }

            continue;
        }
  if (index < MAX_INPUT_LENGTH - 1) {

            buffer[index] = c;

            index++;

            buffer[index] = '\0';
            char text[2];

            text[0] = c;
            text[1] = '\0';


            /*
             * Показываем символ.
             */

            fb_print(
                text,
                FB_WHITE,
                FB_BLACK
            );
        }
    }
}

static CommandType identify_command(char *command)
{
    if (strcmp_local(command, "help") == 0) {
        return HELP;
    }

    if (strcmp_local(command, "echo") == 0) {
        return ECHO;
    }

    if (strcmp_local(command, "reboot") == 0) {
        return REBOOT;
    }

    if (strcmp_local(command, "shutdown") == 0) {
        return SHUTDOWN;
    }

    if (strcmp_local(command, "clear") == 0) {
        return CLEAR;
    }

    return COMMAND_UNKNOWN;
}

static CommandType parse_command(
    char *input,
    char *args[MAX_ARGS_COUNT]
)
{
    unsigned int i = 0;
    unsigned int arg_count = 0;

    CommandType type;

    while (input[i] == ' ') {
        i++;
    }

    if (input[i] == '\0') {
        return COMMAND_UNKNOWN;
    }


    while (
        input[i] != ' ' &&
        input[i] != '\0'
    ) {
        i++;
    }

    if (input[i] == ' ') {

        input[i] = '\0';

        i++;
    }

    type = identify_command(input);
    
    while (
        input[i] != '\0' &&
        arg_count < MAX_ARGS_COUNT
    ) {


        while (input[i] == ' ') {
            i++;
        }


        if (input[i] == '\0') {
            break;
        }

        args[arg_count] = &input[i];

        arg_count++;
        while (
            input[i] != ' ' &&
            input[i] != '\0'
        ) {
            i++;
        }

        if (input[i] == ' ') {

            input[i] = '\0';

            i++;
        }
    }

    while (arg_count < MAX_ARGS_COUNT) {

        args[arg_count] = 0;

        arg_count++;
    }


    return type;
}

static void clear_screen(void)
{
    unsigned int i;

    for (i = 0; i < 80 * 25; i++) {

        fb_write_cell(
            i * 2,
            ' ',
            FB_LIGHT_GREY,
            FB_BLACK
        );
    }

    cursor_pos = 0;

    fb_move_cursor(cursor_pos);
}

void ParseCommand(
    CommandType type,
    char *args[MAX_ARGS_COUNT]
)
{
    switch (type) {

        case HELP:

            fb_println(
                "Available commands:",
                FB_WHITE,
                FB_BLACK
            );

            fb_println(
                "help     - show this message",
               FB_WHITE,
                FB_BLACK
            );

            fb_println(
                "echo     - print text",
                FB_WHITE,
                FB_BLACK
            );

            fb_println(
                "clear    - clear screen",
                FB_WHITE,
                FB_BLACK
            );

            fb_println(
                "reboot   - restart the computer",
                FB_WHITE,
                FB_BLACK
            );

            fb_println(
                "shutdown - shut down the computer",
                FB_WHITE,
                FB_BLACK
            );

            break;
        case ECHO:

            if (args[0] == 0) {

                fb_println(
                    "",
                    FB_WHITE,
                    FB_BLACK
                );

                break;
            }


            for (
                unsigned int i = 0;
                i < MAX_ARGS_COUNT && args[i] != 0;
                i++
            ) {

                fb_print(
                    args[i],
                    FB_WHITE,
                    FB_BLACK
                );


                if (args[i + 1] != 0) {

                    fb_print(
                        " ",
                        FB_WHITE,
                        FB_BLACK
                    );
                }
            }


            fb_println(
                "",
                FB_WHITE,
                FB_BLACK
            );

            break;

        case CLEAR:

            clear_screen();

            break;

        case REBOOT:

            fb_println(
                "Rebooting...",
                FB_GREEN,
                FB_BLACK
            );

            while (inb(0x64) & 0x02) {
            }

            outb(
                0x64,
                0xFE
            );

         while (1) {

                __asm__ volatile (
                    "cli; hlt"
                );
            }

            break;

        case SHUTDOWN:

            fb_println(
                "Shutting down...",
                FB_RED,
                FB_BLACK
            );

            outw(
                0x604,
                0x2000
            );

            outw(
                0xB004,
                0x2000
            );

            while (1) {

                __asm__ volatile (
                    "cli; hlt"
                );
            }

            break;

        case COMMAND_UNKNOWN:
            break;


        default:

            fb_println(
                "Unknown command.",
                FB_LIGHT_RED,
                FB_BLACK
            );

            break;
    }
}

void ShellProcess(void)
{
    char command[MAX_INPUT_LENGTH];

    char *args[MAX_ARGS_COUNT];


    while (1) {

        fb_print(
            path,
            FB_LIGHT_BLUE,
            FB_BLACK
        );

        gets(command);

        CommandType type = parse_command(
            command,
            args
        );

        ParseCommand(
            type,
            args
        );
    }
}
