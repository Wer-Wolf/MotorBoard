#include <string.h>

/*2 byte Command, 2 byte args, 4 byte Antwort*/

#define COMMAND_COUNT 5

void test(volatile uint8_t *arguments) {
    set(&command_buffer.command[0], "ECHO", 4);
}

void get_mmode(volatile uint8_t *arguments) {
    set(&command_buffer.command[0], "GETM", 4);
}

void set_mmode(volatile uint8_t *arguments) {
    set(&command_buffer.command[0], "SETM", 4);
}

void get_rpm(volatile uint8_t *arguments) {
    set(&command_buffer.command[0], "GETR", 4);
}

void start_rpm(volatile uint8_t *arguments) {
    set(&command_buffer.command[0], "STRT", 4);
}

const command_list_t command_list[COMMAND_COUNT] = {
    {.callback = &test, .string = {'A', 'T'}},
    {.callback = &get_mmode, .string = {'G', 'M'}},
    {.callback = &set_mmode, .string = {'S', 'M'}},
    {.callback = &get_rpm, .string = {'G', 'R'}},
    {.callback = &start_rpm, .string = {'S', 'R'}}
};