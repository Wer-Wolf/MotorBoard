#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/atomic.h>

#define SENSOR_ENABLE_PIN PA0

#define capture_active() (TIMSK1 & (1 << ICIE1))

/* Rising Edge (= Auschnitt beginnt) */
#define ACSR_DEFAULT ((1 << ACD) | (1 << ACIC) | (1 << ACIS1) | (1 << ACIS0))

/* Falling Edge (= Ausschnitt endet), Prescaler 64, Wert - 5 */
#define TCCR1B_DEFAULT ((1 << ICNC1) | (1 << CS11) | (1 << CS10))

/* Overflow und Capture Interrupt */
#define TIMSK1_DEFAULT ((1 << ICIE1) | (1 << TOIE1))

static volatile uint16_t capture_value = 0; //Atomar abfragen!
//Bleibt bei Overflow auf 0xFFFF stehen

uint16_t get_capture_value() {
    uint16_t value;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        value = capture_value;
    }
    return value;
}

void start_capture() {
    PORTA |= (1 << SENSOR_ENABLE_PIN); //DDrx anpassen!
    power_adc_enable();
    ACSR = ACSR_DEFAULT & ~(1 << ACD);
    ACSR = ACSR_DEFAULT & ~(1 << ACD) & (1 << ACIE);
}

void stop_capture() {
    ACSR = ACSR_DEFAULT;
    ACSR = ACSR_DEFAULT & (1 << ACI); //Flag löschen
    power_adc_disable();
    TIMSK1 = TIMSK1_DEFAULT & ~(1 << ICIE1); //Interrupt deaktivieren
    /* Counter stoppen */
    TCCR1B = TCCR1B_DEFAULT & ~(1 << CS11) & ~(1 << CS10);
    TIFR1 = (1 << TOV1) | (1 << ICF1); //Flags löschen
    power_timer1_disable();
    PORTA &= ~(1 << SENSOR_ENABLE_PIN);
}

inline void rpm_init() {
    //Komparator initialisieren (Interrupt um Timer zu starten)
    DIDR0 |= (1 << ADC2D) | (1 << ADC1D);
    ACSR = ACSR_DEFAULT;
    power_adc_disable();
    /* Counter stoppen */
    TCCR1B = TCCR1B_DEFAULT & ~(1 << CS11) & ~(1 << CS10);
    //Erst nur Overflow
    TIMSK1 = TIMSK1_DEFAULT & ~(1 << ICIE1);
    power_timer1_disable();
}

ISR(ANA_COMP_vect) {
    if(!capture_active()) { //Prellungen kompensieren
        power_timer1_enable();
        TCNT1 = 0x0000;
        TIFR1 = (1 << ICF1);
        TIMSK1 = TIMSK1_DEFAULT; //Interrupt aktivieren
        TCCR1B = TCCR1B_DEFAULT; //Timer starten
    }
}

ISR(TIM1_CAPT_vect) {
    capture_value = ICR1;
    stop_capture();
}

ISR(TIM1_OVF_vect) {
    capture_value = 0xFFFF; //Overflow
    stop_capture();
}