#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/atomic.h>

#define capture_active() (TIMSK1 & (1 << ICIE1))

static volatile uint16_t capture_value = 0; //Atomar abfragen!
//Bleibt bei Overflow auf 0xFFFF stehen

uint16_t get_capture_value() {
    uint16_t value;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        value = capture_value;
    }
    return value;
}

inline void rpm_init() {
    //Komparator initialisieren (Interrupt um Timer zu starten)
    DIDR0 |= (1 << ADC2D) | (1 << ADC1D);
    ACSR |= (1 << ACIC) | (1 << ACIE) | (1 << ACIS0) | (1 << ACIS1);
    //16-Bit Timer initialisieren (Presacler 64), Wert -5
    TCCR1B |= (1 << ICNC1);
    //Overflow (und Capture)
    TIMSK1 |= (1 << TOIE1);
    power_timer1_disable();
}

ISR(ANA_COMP_vect) {
    if(!capture_active()) { //Prellungen kompensieren
        power_timer1_enable();
        TCNT1 = 0x0000;
        TIFR1 |= (1 << ICF1);
        TIMSK1 |= (1 << ICIE1); //Interrupt aktivieren
        TCCR1B |= (1 << CS11) | (1 << CS10); //Timer starten
    }
}

ISR(TIM1_CAPT_vect) {
    capture_value = ICR1;
    TCCR1B &= ~((1 << CS11) | (1 << CS10)); //Timer stoppen
    TIFR1 |= (1 << TOV1); //Oveflow-Flag löschen
    power_timer1_disable();
}

ISR(TIM1_OVF_vect) {
    TIMSK1 &= ~(1 << ICIE1); //Interrupt deaktivieren
    capture_value = 0xFFFF;
    TCCR1B &= ~((1 << CS11) | (1 << CS10)); //Timer stoppen
    power_timer1_disable();
}