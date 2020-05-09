#include <stdbool.h>

#include <avr/io.h>
#include <avr/fuse.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/atomic.h>

FUSES = 
{
    .low = (FUSE_SUT0 & FUSE_CKSEL3 & FUSE_CKSEL2 & FUSE_CKSEL0),
    .high = (FUSE_SPIEN & FUSE_BODLEVEL1 & FUSE_BODLEVEL0),
    .extended = EFUSE_DEFAULT,
};

#define SPI_MODE 0
#include "../lib/usi_spi.h"
#include "../lib/rpm.h"
#include "../lib/motor.h"

typedef void (*callback_t) (volatile uint8_t *);

#define COMMAND_LENGTH 2 //Rest sind Argumente

typedef struct {
    callback_t callback;
    uint8_t string[COMMAND_LENGTH];
} command_list_t;

typedef union {
    uint32_t frequency; //4 Bytes
    uint8_t command[4];
} buffer;

volatile buffer command_buffer = {0};

uint8_t compare(volatile uint8_t *src, const uint8_t *data, const uint8_t count) {
    uint8_t status = 0;
    for(uint8_t i = 0; i < count; i++) {
        if(*(src + i) != *(data +i)) {
            status = 1;
            break;
        }
    }
    return status;
}

void set(volatile uint8_t *des, const char *string, const uint8_t count) {
    for(uint8_t i = 0; i < count; i++) {
        if(*(string + i) != '\0') {
            *(des + i) = *(string + i);
        } else {
            break;
        }
    }
}

#include "../util/commands.h"

int main(void) {
    DDRA |= (1 << SENSOR_ENABLE_PIN) | (1 << L293D_1A) | (1 << L293D_2A);
    DDRB |= (1 << L293D_3A) | (1 << L293D_4A);
    power_timer0_disable();
    set_sleep_mode(SLEEP_MODE_IDLE);
    //rpm_init();
    usi_spi_init();
    sei(); //Für SPI
    while(true) {
        spi_allow_transmission(&command_buffer.command[0], 4);
        PORTB |= (1 << PB0);
        while(spi_buffer != NULL);
        PORTB &= ~(1 << PB0);
        for(uint8_t i = 0; i < COMMAND_COUNT + 1; i++) {
            if(i == COMMAND_COUNT) { //Kein Befehl wurde erkannt
                set(&command_buffer.command[0], "????", 4);
            } else {
                if(compare(&command_buffer.command[0], &command_list[i].string[0], COMMAND_LENGTH) == 0) {
                    command_list[i].callback(&command_buffer.command[COMMAND_LENGTH]);
                    break;
                }
            }
        }
    }
    return 0;
}