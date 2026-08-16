#include "shell.h"
#include "print.h"
#include "io.h"


#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64

#define HISTORY_SIZE 10

/*
 * Специальные значения, которые getc()
 * возвращает для стрелок.
 *
 * Они не являются обычными ASCII-символами.
 */
#define KEY_UP    0x11
#define KEY_DOWN  0x12


char *path = "home/user/shell>";


/* =========================================================
 * COMMAND HISTORY
 * ========================================================= */

static char history[HISTORY_SIZE][MAX_INPUT_LENGTH];

static unsigned int history_count = 0;

/*
 * Текущая позиция при перемещении по истории.
 *
 * history_count означает "пустая строка после последней
 * команды".
 */
static unsigned int history_position = 0;

/*
 * Здесь сохраняем строку, которую пользователь набрал
 * перед первым нажатием стрелки вверх.
 */
static char history_temp[MAX_INPUT_LENGTH];


/* =========================================================
 * KEYBOARD MAP
 * ========================================================= */

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


/* =========================================================
 * STRING COPY
 * ========================================================= */

static void strcpy_local(char *destination, const char *source)
{
    unsigned int i = 0;

    while (source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }

    destination[i] = '\0';
}


/* =========================================================
 * STRING COMPARE
 *
 * 0 = одинаковые
 * 1 = разные
 * ========================================================= */

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


/* =========================================================
 * KEYBOARD SCAN CODE
 * ========================================================= */

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


/* =========================================================
 * GETC
 *
 * Получить один символ.
 *
 * Дополнительно:
 *
 * KEY_UP   -> стрелка вверх
 * KEY_DOWN -> стрелка вниз
 * ========================================================= */

char getc(void)
{
    unsigned char scancode;
    unsigned char extended;

    while (1) {

        scancode = keyboard_read_scancode();

        /*
         * Extended scan code.
         *
         * Стрелки идут как:
         *
         * E0 48 = UP
         * E0 50 = DOWN
         */

        if (scancode == 0xE0) {

            extended = keyboard_read_scancode();

            /*
             * UP
             */

            if (extended == 0x48) {
                return KEY_UP;
            }

            /*
             * DOWN
             */

            if (extended == 0x50) {
                return KEY_DOWN;
            }

            /*
             * Другие extended keys пока игнорируем.
             */

            continue;
        }


        /*
         * Отпускание клавиши.
         */

        if (scancode & 0x80) {
            continue;
        }


        /*
         * Неизвестный scan code.
         */

        if (scancode >= 128) {
            continue;
        }


        /*
         * Преобразуем scan code в ASCII.
         */

        if (keyboard_map[scancode] != 0) {
            return keyboard_map[scancode];
        }
    }
}


/* =========================================================
 * CLEAR CURRENT INPUT
 *
 * Удаляет текущую строку ввода с экрана.
 * ========================================================= */

static void clear_input(char *buffer)
{
    unsigned int length = 0;

    while (buffer[length] != '\0') {
        length++;
    }

    /*
     * Стираем все символы.
     */

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


/* =========================================================
 * HISTORY SAVE
 * ========================================================= */

static void history_add(const char *command)
{
    unsigned int i;

    /*
     * Пустые команды в историю не добавляем.
     */

    if (command[0] == '\0') {
        return;
    }


    /*
     * Если история заполнена,
     * сдвигаем старые команды вверх.
     */

    if (history_count >= HISTORY_SIZE) {

        for (i = 1; i < HISTORY_SIZE; i++) {
            strcpy_local(
                history[i - 1],
                history[i]
            );
        }

        history_count = HISTORY_SIZE - 1;
    }


    /*
     * Добавляем новую команду в конец.
     */

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


/* =========================================================
 * GET HISTORY COMMAND
 * ========================================================= */

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


/* =========================================================
 * GETS
 *
 * ENTER:
 *     завершить строку
 *
 * BACKSPACE:
 *     удалить символ
 *
 * UP:
 *     предыдущая команда
 *
 * DOWN:
 *     следующая команда
 * ========================================================= */

void gets(char *buffer)
{
    unsigned int index = 0;

    char c;

    /*
     * Новый ввод начинается с пустого буфера.
     */

    buffer[0] = '\0';

    history_position = history_count;

    history_temp[0] = '\0';

    while (1) {

        c = getc();


        /* -------------------------------------------------
         * ENTER
         * ------------------------------------------------- */

        if (c == '\n') {

            buffer[index] = '\0';


            /*
             * Добавляем команду в историю.
             */

            history_add(buffer);


            /*
             * Переход на следующую строку.
             */

            fb_println(
                "",
                FB_WHITE,
                FB_BLACK
            );

            return;
        }


        /* -------------------------------------------------
         * BACKSPACE
         * ------------------------------------------------- */

        if (c == '\b') {

            if (index > 0) {

                index--;

                buffer[index] = '\0';

                cursor_pos--;


                /*
                 * Стираем символ.
                 */

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


        /* -------------------------------------------------
         * UP
         * ------------------------------------------------- */

        if (c == KEY_UP) {

            /*
             * История пуста.
             */

            if (history_count == 0) {
                continue;
            }


            /*
             * Первый UP.
             *
             * Сохраняем текущую строку пользователя.
             */

            if (history_position == history_count) {

                strcpy_local(
                    history_temp,
                    buffer
                );
            }


            /*
             * Идём к предыдущей команде.
             */

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


        /* -------------------------------------------------
         * DOWN
         * ------------------------------------------------- */

        if (c == KEY_DOWN) {

            /*
             * Если мы уже внизу истории,
             * ничего делать не надо.
             */

            if (history_position >= history_count) {
                continue;
            }


            history_position++;


            /*
             * Если дошли после последней команды,
             * восстанавливаем строку, которая была
             * до нажатия UP.
             */

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

                /*
                 * Загружаем следующую команду.
                 */

                history_load(
                    buffer,
                    history_position
                );
            }


            /*
             * Пересчитываем длину строки.
             */

            index = 0;

            while (buffer[index] != '\0') {
                index++;
            }

            continue;
        }


        /* -------------------------------------------------
         * Обычный символ
         * ------------------------------------------------- */

        if (index < MAX_INPUT_LENGTH - 1) {

            buffer[index] = c;

            index++;

            buffer[index] = '\0';


            /*
             * Временная строка из одного символа.
             */

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


/* =========================================================
 * IDENTIFY COMMAND
 * ========================================================= */

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


/* =========================================================
 * PARSE COMMAND
 *
 * Например:
 *
 * echo hello world
 *
 * input:
 *     echo hello world
 *
 * command:
 *     echo
 *
 * args[0]:
 *     hello
 *
 * args[1]:
 *     world
 * ========================================================= */

static CommandType parse_command(
    char *input,
    char *args[MAX_ARGS_COUNT]
)
{
    unsigned int i = 0;
    unsigned int arg_count = 0;

    CommandType type;


    /*
     * Пропускаем начальные пробелы.
     */

    while (input[i] == ' ') {
        i++;
    }


    /*
     * Пустая команда.
     */

    if (input[i] == '\0') {
        return COMMAND_UNKNOWN;
    }


    /*
     * Ищем конец имени команды.
     */

    while (
        input[i] != ' ' &&
        input[i] != '\0'
    ) {
        i++;
    }


    /*
     * Отделяем команду от аргументов.
     */

    if (input[i] == ' ') {

        input[i] = '\0';

        i++;
    }


    /*
     * Определяем команду.
     */

    type = identify_command(input);


    /*
     * Разбираем аргументы.
     */

    while (
        input[i] != '\0' &&
        arg_count < MAX_ARGS_COUNT
    ) {

        /*
         * Пропускаем пробелы.
         */

        while (input[i] == ' ') {
            i++;
        }


        /*
         * Конец строки.

         */

        if (input[i] == '\0') {
            break;
        }


        /*
         * Запоминаем начало аргумента.
         */

        args[arg_count] = &input[i];

        arg_count++;


        /*
         * Ищем конец аргумента.
         */

        while (
            input[i] != ' ' &&
            input[i] != '\0'
        ) {
            i++;
        }


        /*
         * Завершаем аргумент.
         */

        if (input[i] == ' ') {

            input[i] = '\0';

            i++;
        }
    }


    /*
     * Остальные аргументы = NULL.
     */

    while (arg_count < MAX_ARGS_COUNT) {

        args[arg_count] = 0;

        arg_count++;
    }


    return type;
}


/* =========================================================
 * CLEAR SCREEN
 * ========================================================= */

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


/* =========================================================
 * PARSE COMMAND EXECUTION
 * ========================================================= */

void ParseCommand(
    CommandType type,
    char *args[MAX_ARGS_COUNT]
)
{
    switch (type) {


        /* -------------------------------------------------
         * HELP
         * ------------------------------------------------- */

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


        /* -------------------------------------------------
         * ECHO
         * ------------------------------------------------- */

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


        /* -------------------------------------------------
         * CLEAR
         * ------------------------------------------------- */

        case CLEAR:

            clear_screen();

            break;


        /* -------------------------------------------------
         * REBOOT
         * ------------------------------------------------- */

        case REBOOT:

            fb_println(
                "Rebooting...",
                FB_GREEN,
                FB_BLACK
            );


            /*
             * Ждём освобождения input buffer.
             */

            while (inb(0x64) & 0x02) {
            }


            /*
             * Reset через PS/2 controller.
             */

            outb(
                0x64,
                0xFE
            );


            /*
             * Если reset не произошёл.
             */

            while (1) {

                __asm__ volatile (
                    "cli; hlt"
                );
            }

            break;


        /* -------------------------------------------------
         * SHUTDOWN
         * ------------------------------------------------- */

        case SHUTDOWN:

            fb_println(
                "Shutting down...",
                FB_RED,
                FB_BLACK
            );


            /*
             * QEMU / Bochs.
             */

            outw(
                0x604,
                0x2000
            );

            outw(
                0xB004,
                0x2000
            );


            /*
             * Если выключение не произошло.
             */

            while (1) {

                __asm__ volatile (
                    "cli; hlt"
                );
            }

            break;


        /* -------------------------------------------------
         * UNKNOWN
         * ------------------------------------------------- */

        case COMMAND_UNKNOWN:

            /*
             * Пустая команда просто возвращает prompt.
             */

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


/* =========================================================
 * SHELL PROCESS
 * ========================================================= */

void ShellProcess(void)
{
    char command[MAX_INPUT_LENGTH];

    char *args[MAX_ARGS_COUNT];


    while (1) {

        /*
         * Prompt.
         */

        fb_print(
            path,
            FB_LIGHT_BLUE,
            FB_BLACK
        );


        /*
         * Получаем команду.
         */

        gets(command);


        /*
         * Разбираем команду.

         */

        CommandType type = parse_command(
            command,
            args
        );


        /*
         * Выполняем команду.
         */

        ParseCommand(
            type,
            args
        );
    }
}
