/*
 * Midterm1.c
 *
 * Created: 10/27/2019 9:55:47 PM
 * Author : bruce
 */ 


/*
	This program uses the Analog to Digital Converter (ADC) of the ATmega328p along with
	the USART and Timer1 to convert the analog signal from an LM35DZ (Temperature Sensor in Celsius)
	into a digital one, convert that data into a readable temperature value, and then transmit
	said data to a channel through TeamSpeak via an ESP8266-01 Wi-Fi Module every 15-20 seconds.
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
//unsigned int number = 65;					//Int declared for UART
//double n = 1.3689;							//Double/Float declared for UART
char String_num[];							//String array to hold an int after converting to string for int
char String_flt[];							//String array to hold a double value after converting to string
volatile uint8_t time_ovf;					//Integer to hold the amount of times the timer overflows 
float temperature;							//Float variable used to hold the measured analog value from the LM35DZ
char Degrees[]="° F";						//Used to make the output temperature value readable as Fahrenheit 
char CIPMUX[]="AT+CIPMUX=1";				//AT command string for the ESP8266 for setting up a Single or Multi-line IP connection (value of '0' for single)
char CWJAP[]="AT+CWJAP=\"";					//AT command string for the ESP8266 used to scan for available access points
char SSID[]="\Your SSID\",";				//A character array used to hold the name of the access point the user wishes to connect to
char Password[]="\"Your Password"";			//A character array used to hold the password for the access point the user wishes to connect to.

char CIPSTART[]="AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80";  //AT command string for the ESP8266 used to start a connection with ThingSpeak in TCP mode
char CIPSEND[]="AT+CIPSEND=4,77";								 //AT command used to send a number of bits through the ESP8266 to ThingSpeak
char TS_COMMAND[]="GET /update?api_key=";					     //Character array used to the first part of the API key command for a ThingSpeak channel
char API_KEY[]="C3ULAMG0GEL8PX8E&field1=";						 //Character array that holds the ThingSpeak channel API key
char CIPCLOSE[]="AT+CIPCLOSE";									 //AT command string to close the connection with ThingSpeak after data transmission

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
/*

//Optional function if the average of 10 temperature readings is to be sent to ThingSpeak
void average_temp(){
	
	int temp_array[10];
	int average_sum = 0;
	
	for(int i=0;i<10;i++){
		
		ADC_Read(0);
		
		temp_array[i] = temperature;
		temperature = 0;
		
	}
	
	for(int n=0;n<10;n++){
		
		average_sum += temp_array[n];
		
	}
	average_sum = average_sum/10;
	temperature = ((average_sum/2)*1.8)+32;
	average_sum = 0;
	
}
*/
int main(void)
{
	initialize_UART(); //Initializes UART
	timer1_init();		//Initializes Timer1
	ADC_init();			//Initialize ADC
    ADC_Read(0);		//Do one conversion (slowest conversion) for a table conversion process during the main loop.
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ESP8266-01 Initialization
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	USART_putstring(CIPMUX);
	USART_putstring(LineBreak);
	_delay_ms(500);
	
	USART_putstring(CWJAP);
	USART_putstring(SSID);
	USART_putstring(Password);
	USART_putstring(LineBreak);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
    while (1) 
    {
		
		if(time_ovf >= 130){ //Uses increments of 4 for a timer1 overflow interrupt of roughly 1 second. (130 is roughly 33 seconds)
			
			//Note: This portion of the code is supposed to show the temperature before sending it over the ESP
			//however, I think it causes the ESP to send out an "Error" msg after every AT command following the
			//initial temperature output.
			ADC_Read(0); //Using ADC 0
			USART_putstring(String);
			USART_putflt(String_num, temperature);
			USART_putstring(Degrees);
			USART_putstring(LineBreak);
			USART_putstring(LineBreak);
			_delay_ms(2000);
			
			//This portion of the loop readies the ESP to send data through it.
			USART_putstring(CIPSTART);
			USART_putstring(LineBreak);
			
			_delay_ms(1000);
			
			// This portion of the code is used to send the temperature data to ThingSpeak
			//however, the ESP does not give a confirmation of the sent data unless it's
			//sent twice.
			//Note: If one of the TS_COMMAND blocks is removed it still sends data on the initial loop
			//but does not give confirmation.
			USART_putstring(CIPSEND);
			USART_putstring(LineBreak);
			
			_delay_ms(2000);
			
			USART_putstring(TS_COMMAND);
			USART_putstring(API_KEY);
			USART_putflt(String_num,temperature);
			USART_putstring(LineBreak);
			
			_delay_ms(20000);
			
			
			USART_putstring(TS_COMMAND);
			USART_putstring(API_KEY);
			USART_putflt(String_num,temperature);
			USART_putstring(LineBreak);
			
			_delay_ms(2000);
			
			
			//Closes the ESP after data is sent
			USART_putstring(CIPCLOSE);
			USART_putstring(LineBreak);
			
			_delay_ms(2000);
			
			
			TCNT1 = 0;
			time_ovf = 0;
			
		}
		
		else;
		
    }
}



