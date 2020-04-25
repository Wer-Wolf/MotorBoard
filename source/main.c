#include <string.h>
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

#define COMMAND_SIZE 4

#define COMMAND_TEST "TEST"

typedef union {
    uint32_t frequency; //4 Bytes
    uint8_t command[4];
} buffer;

buffer command_buffer = {.command[0] = 'E', .command[1] = 'C', .command[2] = 'H', .command[3] = 'O'};

inline void sleep() {
    sleep_enable();
    NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE) {
        sleep_cpu();
    }
    sleep_disable();
}

int main(void) {
    DDRA |= (1 << SENSOR_ENABLE_PIN) | (1 << L293D_1A) | (1 << L293D_2A);
    DDRB |= (1 << L293D_3A) | (1 << L293D_4A);
    power_timer0_disable();
    set_sleep_mode(SLEEP_MODE_IDLE);
    //rpm_init();
    usi_spi_init();
    sei(); //Für SPI
    while(true) {
        /*ATOMIC_BLOCK(ATOMIC_FORCEON) {
            while(!SPI_DATA_READY) {
                sleep();
            }
        }*/
        spi_allow_transmission(&command_buffer.command[0], 4);
        PORTB |= (1 << PB0);
        while(spi_buffer != NULL);
        PORTB &= ~(1 << PB0);
        /*command_buffer.command[0] = 'E';
        command_buffer.command[1] = 'C';
        command_buffer.command[2] = 'H';
        command_buffer.command[3] = 'O';*/
        memcpy(&command_buffer.command[0], "ECHO", 4);
        /*if(memcmp(&command_buffer.command[0], "TEST", 4) == 0) {
            PORTA |= (1 << PA7);
            memcpy(&command_buffer.command[0], "ECHO", 4);
        } else {
            PORTA &= ~(1 << PA7);
            memcpy(&command_buffer.command[0], "FAIL", 4);
        }*/
        /*ATOMIC_BLOCK(ATOMIC_FORCEON) {
            while(spi_allow_transmission(command_buffer.command, 4) != SPI_SUCCESS) {
                sleep(); //Warte bis Übertragung beendet
            }
        }*/
    }
    return 0;
}