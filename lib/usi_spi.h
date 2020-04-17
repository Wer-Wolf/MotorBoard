#include <stddef.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h> //Vlt.?
#include <util/atomic.h> //C99

#ifndef USI_SPI_MODE
    #define USI_SPI_MODE 0 //Standard
#endif

#define SPI_SUCCESS 0
#define SPI_FAIL 1

#define SPI_DATA_READY (spi_buffer_ready == true)

static volatile uint8_t *spi_buffer = NULL; //Zeiger auf den momentanen Puffer

static volatile size_t spi_buffer_offset = 0; //Aktueller Ort im Puffer

static volatile uint8_t *spi_buffer_max = NULL; //Zeiger auf das Ende des Puffers

/*Darf nicht von Außen verändert werden!*/
volatile bool spi_buffer_ready = true; //Puffer kann mit Daten befüllt werden

/*Damit ein leerer Puffer wärend einer Übertragung gefüllt werden kann*/
static volatile bool spi_buffer_valid = false;

inline void usi_spi_init() {
    //USI Overflow (DDRx anpassen)
    #if USI_SPI_MODE == 0
    USICR |= (1 << USIWM0) | (1 << USICS1);
    #else //Mode 1
    USICR |= (1 << USIWM0) | (1 << USICS1) | (1 << USICS0);
    #endif
    //Pinchange auf PB2 (PCINT10)
    PCMSK1 |= (1 << PCINT10);
    GIMSK |= (1 << PCIE1);
}

uint8_t spi_allow_transmission(uint8_t *buffer ,size_t byte_count) {
    uint8_t status;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //Falls mitendrin Übertragung beginnt
        if(spi_buffer_ready == true) { //Puffer wird nicht genutzt
            spi_buffer_max = buffer + byte_count; //Neue Größe des Puffers
            spi_buffer = buffer; //Neue Daten
            spi_buffer_ready = false;
            status = SPI_SUCCESS;
        } else {
            status = SPI_FAIL; //Es liegen bereits Daten im Puffer
        }
    }
    return status;
}

ISR(PCINT1_vect) {
    if(PORTB & (1 << PB2)) { //Steigende Flanke, Übertragung beginnt
        USISR |= (1 << USIOIF) | (16 << USICNT0); //USI reset
        USICR |= (1 << USIOIE);
        if(spi_buffer_ready == false) { //Puffer enthält Daten
            spi_buffer_valid = true; //Puffer wird versendet
            spi_buffer_offset = 0;
        } else {
            spi_buffer_valid = false; //Puffer wird ignoriert (weil enthält keine Daten)
        }
    } else {
        USICR &= ~(1 << USIOIE); //USI abschalten
        spi_buffer_ready = true;
    }
}

ISR(USI_OVF_vect) {
    USISR |= (1 << USIOIF); //Flag löschen
    uint8_t input = USIDR;
    if(spi_buffer_valid == true) { //Daten zum versenden bereit
        USIDR = *(spi_buffer + spi_buffer_offset);
        *(spi_buffer + spi_buffer_offset) = input; //Replace old data with new input data
        spi_buffer_offset++;
    } else {
        USIDR = 0;
    }
}