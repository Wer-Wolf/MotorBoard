#include <string.h>
#include <stdlib.h>

/* 2 byte Command, 2 byte args, 4 byte Antwort */

#define COMMAND_COUNT 5

#define FIRST_ARG *(arguments)
#define SECOND_ARG *(arguments + 1)

/* S = Stop, L/R = Left/Right */
static char motor_state[2] = {'S', 'S'};

/*
 * Keine Argumente
 */

void test(volatile uint8_t *arguments) {
    set(&command_buffer.command[0], "Test", 4);
}

/*
 * 1. ASCII 0
 * 2. ASCII Motor-ID
 */

void get_mmode(volatile uint8_t *arguments) {
    //set(&command_buffer.command[0], "GETM", 4);

    if(FIRST_ARG != '0' || SECOND_ARG >= 'C' || SECOND_ARG < 'A') {
        set(&command_buffer.command[0], "WngM", 4);
    } else {
        uint8_t motor_id;

        if(SECOND_ARG == 'A') {
            motor_id = 0;
        } else {
            motor_id = 1;
        }
        switch(motor_state[motor_id]) {
            case 'S':
                set(&command_buffer.command[0], "Stop", 4);
                break;
            case 'L':
                set(&command_buffer.command[0], "Left", 4);
                break;
            case 'R':
                set(&command_buffer.command[0], "Rght", 4);
                break;
            default:
                set(&command_buffer.command[0], "Err!", 4);
        }
    }
}

/*
 * 1. Motor_ID
 * 2. Richtung (L, R oder S)
 */

void set_mmode(volatile uint8_t *arguments) {
    //set(&command_buffer.command[0], "SETM", 4);
    if(FIRST_ARG >= 'C' || FIRST_ARG < 'A') {
        set(&command_buffer.command[0], "WngM", 4);
    } else {
        switch(SECOND_ARG) {
            case 'S':
                if(FIRST_ARG == 'A') {
                    MOTOR_A_SET_STOP;
                    motor_state[0] = 'S';
                } else {
                    MOTOR_B_SET_STOP;
                    motor_state[1] = 'S';
                }
                break;
            case 'L':
                if(FIRST_ARG == 'A') {
                    MOTOR_A_SET_LEFT;
                    motor_state[0] = 'L';
                } else {
                    MOTOR_B_SET_LEFT;
                    motor_state[1] = 'L';
                }
                break;
            case 'R':
                if(FIRST_ARG == 'A') {
                    MOTOR_A_SET_RIGHT;
                    motor_state[0] = 'R';
                } else {
                    MOTOR_B_SET_RIGHT;
                    motor_state[1] = 'R';
                }
                break;
            default:
                set(&command_buffer.command[0], "WngV", 4);
                return;
            }
        set(&command_buffer.command[0], "OKOK", 4);
    }
}

void get_rpm(volatile uint8_t *arguments) {
    uint16_t value = get_capture_value();
    char output[6] = {'?'};
    itoa(value, output, 10);
    set(&command_buffer.command[0], output, 4);
}

void start_rpm(volatile uint8_t *arguments) {
    start_capture();
    set(&command_buffer.command[0], "OKOK", 4);
}

const command_list_t command_list[COMMAND_COUNT] = {
    {.callback = &test, .string = {'A', 'T'}},
    {.callback = &get_mmode, .string = {'G', 'M'}},
    {.callback = &set_mmode, .string = {'S', 'M'}},
    {.callback = &get_rpm, .string = {'G', 'R'}},
    {.callback = &start_rpm, .string = {'S', 'R'}}
};