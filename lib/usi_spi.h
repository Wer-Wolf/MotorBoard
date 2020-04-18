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

#define SPI_BUFFER_EMPTY (spi_buffer + spi_buffer_offset >= spi_buffer_max)

#define SPI_DATA_READY (spi_buffer == NULL)

volatile uint8_t *spi_buffer = NULL; //Zeiger auf den momentanen Puffer

static volatile size_t spi_buffer_offset = 0; //Aktueller Ort im Puffer

static volatile uint8_t *spi_buffer_max = NULL; //Zeiger auf das Ende des Puffers

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
        if(spi_buffer == NULL && buffer != NULL && byte_count > 0) { //Puffer frei und Parameter valid
            spi_buffer_max = buffer + byte_count; //Neue Größe des Puffers
            spi_buffer = buffer; //Neue Daten
            status = SPI_SUCCESS;
        } else {
            status = SPI_FAIL; //Es liegen bereits Daten im Puffer oder Parameter falsch
        }
    }
    return status;
}

ISR(PCINT1_vect) {
    if(PORTB & (1 << PB2)) { //Steigende Flanke, Übertragung beginnt
        USISR |= (1 << USIOIF) | (16 << USICNT0); //USI reset
        USICR |= (1 << USIOIE);
        if(spi_buffer != NULL) { //Puffer enthält Daten
            spi_buffer_valid = true; //Puffer wird versendet
            spi_buffer_offset = 0;
        } else {
            spi_buffer_valid = false; //Puffer wird ignoriert (weil enthält keine Daten)
        }
    } else {
        USICR &= ~(1 << USIOIE); //USI abschalten
        if(spi_buffer_valid == true) { //Puffer enthielt zuvor Daten
            spi_buffer = NULL; //Puffer wird geleert
        }
    }
}

ISR(USI_OVF_vect) {
    USISR |= (1 << USIOIF); //Flag löschen
    uint8_t input = USIDR;
    if(spi_buffer_valid == true && !SPI_BUFFER_EMPTY) { //Daten zum versenden bereit
        USIDR = *(spi_buffer + spi_buffer_offset);
        *(spi_buffer + spi_buffer_offset) = input; //Replace old data with new input data
        spi_buffer_offset++;
    } else {
        USIDR = 0;
    }
}