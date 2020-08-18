#include <avr/io.h>

#define L293D_1A PA3 //DDRA
#define L293D_2A PA7
#define L293D_3A PB0 //DDRB
#define L293D_4A PB1

/*#define MOTOR_A_STATUS (DDRA & ((1 << L293D_1A) | (1 << L293D_2A)))
#define MOTOR_A_STOP ((1 << L293D_1A) | (1 << L293D_2A))
#define MOTOR_A_LEFT (1 << L293D_1A)
#define MOTOR_A_RIGHT (1 << L293D_2A)*/

#define MOTOR_A_SET_LEFT PORTA |= (1 << L293D_1A); PORTA &= ~(1 << L293D_2A)
#define MOTOR_A_SET_RIGHT PORTA |= (1 << L293D_2A); PORTA &= ~(1 << L293D_1A)
#define MOTOR_A_SET_STOP PORTA &= ~((1 << L293D_1A) | (1 << L293D_2A))

#define MOTOR_B_SET_LEFT PORTB |= (1 << L293D_3A); PORTB &= ~(1 << L293D_4A)
#define MOTOR_B_SET_RIGHT PORTB |= (1 << L293D_4A); PORTB &= ~(1 << L293D_3A)
#define MOTOR_B_SET_STOP PORTB &= ~((1 << L293D_3A) | (1 << L293D_4A))