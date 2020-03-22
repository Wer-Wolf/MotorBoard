#include <avr/io.h>

#define L293D_1A PA3 //DDRA
#define L293D_2A PA7
#define L293D_3A PB0 //DDRB
#define L293D_4A PB1

#define MOTOR_A_SET_LEFT DDRA |= (1 << L293D_1A); DDRA &= ~(1 << L293D_2A)
#define MOTOR_A_SET_RIGHT DDRA |= (1 << L293D_2A); DDRA &= ~(1 << L293D_1A)
#define MOTOR_A_STOP DDRA &= ~((1 << L293D_1A)| (1 << L293D_2A))

#define MOTOR_B_SET_LEFT DDRB |= (1 << L293D_3A); DDRB &= ~(1 << L293D_4A)
#define MOTOR_B_SET_RIGHT DDRB |= (1 << L293D_4A); DDRB &= ~(1 << L293D_3A)
#define MOTOR_B_STOP DDRB &= ~((1 << L293D_3A)| (1 << L293D_4A))