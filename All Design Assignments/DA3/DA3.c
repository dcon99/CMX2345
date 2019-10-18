
/*
	This program is meant to demonstrate basic USART usage of the ATmega328p by transmitting
	a string, a random integer, and a random double number over a serial port every second (roughly)
	by using Timer1.
*/
#define F_CPU 8000000UL						//Sets the clock speed of the MCU
#define BAUD 9600							//Sets the baud rate for the UART to transmit
#define BAUDRATE ((F_CPU) / (BAUD * 8UL)-1) // Set Baud Rate Value for UBRR

/////////////////////////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
/////////////////////////////////////////////////////////////////////////////////////////////////////

char String[]="Hello world!!";				//Character array for string
char LineBreak[]="\r\n";					//New line and return string array for a neater transmission
unsigned int number = 65;					//Int declared for UART
double n = 1.3689;							//Double/Float declared for UART
char String_num[];							//String array to hold an int after converting to string for int
char String_flt[];							//String array to hold a double value after converting to string
volatile uint8_t time_ovf;					//Integer to hold the amount of times the timer overflows 

/////////////////////////////////////////////////////////////////////////////////////////////////////


//A function used to initialize Timer1 of the ATmega328P with a prescale of 64
void timer1_init()
{
	//Set up timer with a prescale of 64
	TCCR1A |= (0<<COM1A1) | (0<<COM1A0);
	TCCR1B |= (1<<CS11)|(1 << CS10);
	
	
	//Initialize counter
	TCNT1 = 0;
	
	//Enable overflow interrupt
	TIMSK1 |= (1 << TOIE1);
	
	//Initialize overflow counter variable
	time_ovf = 0;
}

//Used to initialize the UART of the ATmega328P  with a baud rate of 9600 and enables global interrupts.
void initialize_UART(){

	UBRR0H = (uint8_t)(BAUDRATE>>8);
	UBRR0L = (uint8_t)(BAUDRATE);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = ((1<<UCSZ00)|(1<<UCSZ01));

	sei();
	
}

//Function used to send one character (8-bits) at a time through USART
void USART_send( unsigned char data){
	
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
	
}

//Function used to send converted numbers through a string array through USART
void USART_send_num( unsigned int data){
	
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
	
}

//Function used to receive data from USART
unsigned char USART_receive(void){
	
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
	
}
	
	
//Function used to transmit an array of characters (string) through USART one character at a time.	
void USART_putstring(char* StringPtr){
	
	while(*StringPtr != 0x00){
		USART_send(*StringPtr);
	StringPtr++;}  
	
}

//Function used to transmit an array of characters of an integer converted into a string through USART one character at a time.	
void USART_putnumber(char* String_num){
	
	String_num =  (utoa(number,String_num,10)); //Converts an int to a string
	while(*String_num != 0x00){    
		USART_send_num(*String_num);
	String_num++;} 
	
}

//Function used to transmit an array of characters of an double converted into a string through USART one character at a time.	
void USART_putflt(char* String_flt){
	
	String_flt =  dtostrf(n,0,4,String_flt); //Converts a double to a string
	while(*String_flt != 0x00){    
		USART_send_num(*String_flt);
	String_flt++;}        
	
}

//Interrupt Service Routine used for Timer1 Overflow.
ISR(TIMER1_OVF_vect)
{
	//Keep track of number of overflows
	time_ovf++;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{
	initialize_UART(); //Initializes UART
	timer1_init();		//Initializes Timer1
	
    while (1) 
    {
		
		if(time_ovf >= 4){ 
			
			USART_putstring(String);
			
			USART_putstring(LineBreak);
			USART_putstring(LineBreak);
			
			USART_putnumber(String_num);
			
			USART_putstring(LineBreak);
			USART_putstring(LineBreak);
			
			USART_putflt(String_flt);
			
			USART_putstring(LineBreak);
			USART_putstring(LineBreak);
			
			TCNT1 = 0;
			time_ovf = 0;
			
		}
		
		else;
		
    }
}

