#include <avr/io.h>
#include <avr/fuse.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#include "../lib/usi_spi.h"
#include "../lib/rpm.h"
#include "../lib/motor.h"

int main(void) {
    
    power_timer0_disable();

    return 0;
}