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

#define MISO PA5
#define MOSI PA6
#define SCK PA4
#define SS PB2

#define SPI_BUFFER_AVAILABLE (spi_buffer + spi_buffer_offset <= spi_buffer_max) //Daten können geschrieben/gelesen werden

#define SPI_CURRENT_ITEM *(spi_buffer + spi_buffer_offset)

#define SPI_DATA_READY (spi_buffer == NULL)

//WIP
#define SPI_BUFFER_ITEMS (spi_buffer_offset - 1) //Nur valid nach einer Übertragung und wenn SPI_DATA_READY

volatile uint8_t *spi_buffer = NULL; //Zeiger auf den momentanen Puffer

volatile size_t spi_buffer_offset = 0; //Aktueller Ort im Puffer

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
    MCUCR |= (1 << ISC00);
    GIMSK |= (1 << INT0);
}

uint8_t spi_allow_transmission(uint8_t *buffer ,size_t byte_count) {
    uint8_t status;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //Falls mitendrin Übertragung beginnt
        if(SPI_DATA_READY && buffer != NULL && byte_count > 0) { //Puffer frei und Parameter valid
            spi_buffer_max = buffer + byte_count - 1; //Neue Größe des Puffers
            spi_buffer = buffer; //Neue Daten
            status = SPI_SUCCESS;
        } else {
            status = SPI_FAIL; //Es liegen bereits Daten im Puffer oder Parameter falsch
        }
    }
    return status;
}

ISR(INT0_vect) {
    if(PINB & (1 << PINB2)) { //Steigende Flanke, Übertragung endet
        PORTA |= (1 << PA7);
        USICR &= ~(1 << USIOIE); //USI abschalten
        USISR = (1 << USIOIF);
        if(spi_buffer_valid == true) { //Puffer enthielt zuvor Daten
            spi_buffer = NULL; //Puffer wird geleert
        }
    } else {
        PORTA &= ~(1 << PA7);
        USISR = (1 << USIOIF); //USI reset
        if(spi_buffer != NULL) { //Puffer enthält Daten
            spi_buffer_valid = true; //Puffer wird versendet
            spi_buffer_offset = 0;
            USIDR = SPI_CURRENT_ITEM; //Muss vorgeladen werden
        } else {
            spi_buffer_valid = false; //Puffer wird ignoriert (weil enthält keine Daten)
        }
        USICR |= (1 << USIOIE); //USI starten
    }
}

ISR(USI_OVF_vect) {
    USISR = (1 << USIOIF); //Flag löschen
    uint8_t input = USIDR;
    if(spi_buffer_valid == true) { //Puffer erreichbar
        if(SPI_BUFFER_AVAILABLE) { //Daten passen in Puffer
            SPI_CURRENT_ITEM = input; //Puffer updaten
            spi_buffer_offset++;
            if(SPI_BUFFER_AVAILABLE) { //Puffer ist nicht leer
                USIDR = SPI_CURRENT_ITEM;
            } else {
                USIDR = 'N'; //No Data
            }
        } else {
            USIDR = 'F'; //Buffer full
        }
    } else {
        USIDR = 'I'; //Invalid
    }
}