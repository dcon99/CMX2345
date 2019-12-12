/*
	This program is meant to demonstrate the ADC (Analog-to-Digital Converter) of the
	ATmega328P alongside its UART and Timer1 capabilities. The purpose of this code is 
	to convert the analog value provided by an LM35DZ temperature sensor using the
	ADC and then transmit the newly converted value onto a terminal through UART
	every second (approximately a second) using Timer1. 


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

char String[]="The Current Temperature Is: ";				//Character array for string
char LineBreak[]="\r\n";					//New line and return string array for a neater transmission
char String_num[];							//String array to hold an int after converting to string for int
char String_flt[];							//String array to hold a double value after converting to string
volatile uint8_t time_ovf;					//Integer to hold the amount of times the timer overflows 
float temperature;							//Float variable used to hold the measured analog value from the LM35DZ
char Degrees[]="° F";						//Used to make the output temperature value readable as Fahrenheit 

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
void USART_putnumber(char* String_num, unsigned int temperature){
	
	String_num =  (utoa(temperature,String_num,10)); //Converts an int to a string
	while(*String_num != 0x00){    
		USART_send_num(*String_num);
	String_num++;} 
	
}

//Function used to transmit an array of characters of an double converted into a string through USART one character at a time.	
void USART_putflt(char* String_flt, float temperature){
	
	String_flt =  dtostrf(temperature,0,2,String_flt); //Converts a double to a string
	while(*String_flt != 0x00){    
		USART_send_num(*String_flt);
	String_flt++;}        
	
}

//Function used to initialize the ADC with an adc-prescaler of 64 and using a reference voltage of 5V
void ADC_init(){
	
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0));    //Prescaler at 64 so we have an 125Khz clock source
	ADMUX |= (0<<REFS1)|( 1 << REFS0 );

}

//Interrupt Service Routine used for Timer1 Overflow.
ISR(TIMER1_OVF_vect)
{
	//Keep track of number of overflows
	time_ovf++;
	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

//This function is used to start a single-conversion from an ADC of the user's choice and store the converted value in the temperature variable.
void ADC_Read( uint8_t channel )
{
   // Select the ADC channel to be read.
   ADMUX |= channel;
	 // Turn on the ADC.
   ADCSRA |= ( 1 << ADEN );

   // Start the conversion.
   ADCSRA |= ( 1 << ADSC );

   
   while( ADCSRA & ( 1 << ADIF ) );
  
	temperature= ADC; //read upper 8bits
 
   temperature = ((temperature/2)*1.8)+32; //Convert converted value to Fahrenheit.

}

int main(void)
{
	initialize_UART(); //Initializes UART
	ADC_init();			//Initialize ADC
    ADC_Read(0);		//Do one conversion (slowest conversion) for a table conversion process during the main loop.
	timer1_init();		//Initializes Timer1
	
    while (1) 
    {
		
		if(time_ovf >= 40){ //Uses increments of 4 for a timer1 overflow interrupt of roughly 1 second.
					
			ADC_Read(0); //Using ADC 0
			//USART_putstring(String);
			//USART_putflt(String_num, temperature);
			//USART_putstring(Degrees);
			USART_putstring(LineBreak);
			//USART_putstring(LineBreak);
			
			
			
			TCNT1 = 0;
			time_ovf = 0;
			
		}
		
		else;
		
    }
}