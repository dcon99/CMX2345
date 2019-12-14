/*
	The purpose of this program is to demonstrate how an Atmega328p
	can utilize the I2C/TWI protocol to integrate itself with other
	devices by using an APDS9960 Sensor as an example. What this
	program will do initiate the sensor and read all of the values
	produced by it and send the data to ThingSpeak simultaneously.
	The sensor will read values for RGB and ambient light in general,
	the Atmega328p will be able to display the current values of the sensor
	through the use of its UART, however, it will send the data to
	ThingSPeak once every 15 seconds (Limited to the free-account usage)
	using Timer1. 
	
	-Bruce
 */ 


#define F_CPU 16000000UL					 //Define CPU clock Frequency e.g. here its 8MHz 
#include <avr/io.h>							 //Include AVR std. library file 
#include <util/delay.h>						 //Include delay header file 
#include <inttypes.h>						 //Include integer type header file 
#include <stdlib.h>							 //Include standard library file
#include <stdio.h>							 //Include standard library file
#include "APDS9960_def.h"					 //Include APDS9960 register define file
#include "i2c_master.h"						 //Include I2C Master header file
#include "uart.h"							 //Include USART header file

// Constants
#define LIGHT_INT_HIGH  1000 // High light level for interrupt
#define LIGHT_INT_LOW   10   // Low light level for interrupt

// Global variables

uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;
int isr_flag = 0;
uint16_t threshold = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char String[]="The Current Temperature Is: ";				//Character array for string
char LineBreak[]="\r\n";					//New line and return string array for a neater transmission
//unsigned int number = 65;					//Int declared for UART
//double n = 1.3689;							//Double/Float declared for UART
char String_num[];							//String array to hold an int after converting to string for int
char String_flt[];							//String array to hold a double value after converting to string
volatile uint8_t time_ovf;					//Integer to hold the amount of times the timer overflows 
//float temperature;							//Float variable used to hold the measured analog value from the LM35DZ
char Degrees[]="° F";						//Used to make the output temperature value readable as Fahrenheit 
char CIPMUX[]="AT+CIPMUX=1";				//AT command string for the ESP8266 for setting up a Single or Multi-line IP connection (value of '0' for single)
char CWJAP[]="AT+CWJAP=\"";					//AT command string for the ESP8266 used to scan for available access points
char SSID[]="\Your SSID\",";				//A character array used to hold the name of the access point the user wishes to connect to
char Password[]="\"Your Password";			//A character array used to hold the password for the access point the user wishes to connect to.
char CIPSTART[]="AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80";  //AT command string for the ESP8266 used to start a connection with ThingSpeak in TCP mode
char CIPSEND[]="AT+CIPSEND=4, Number of Bits to Send";								 //AT command used to send a number of bits through the ESP8266 to ThingSpeak
char TS_COMMAND[]="GET /update?api_key=";					     //Character array used to the first part of the API key command for a ThingSpeak channel
char API_KEY[]="C3ULAMG0GEL8PX8E&field1=";						 //Character array that holds the ThingSpeak channel API key
char CIPCLOSE[]="AT+CIPCLOSE";									 //AT command string to close the connection with ThingSpeak after data transmission

char FIELD2[]="&field2=";										//Char array for R value in RGB
char FIELD3[]="&field3=";										//Char array for G value in RGB
char FIELD4[]="&field4=";										//Char array for B value in RGB
char CR_NL[]="\r\n";											//Carrier return and new line

												
//Note: API_KEY array can be used for ambient light value

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void init_uart(uint16_t baudrate){

	uint16_t UBRR_val = (F_CPU/16)/(baudrate-1);

	UBRR0H = UBRR_val >> 8;
	UBRR0L = UBRR_val;

	UCSR0B |= (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0); // UART TX (Transmit - senden) einschalten
	UCSR0C |= (1<<USBS0) | (3<<UCSZ00); //Modus Asynchron 8N1 (8 Datenbits, No Parity, 1 Stopbit)
}

void uart_putc(unsigned char c){

	while(!(UCSR0A & (1<<UDRE0))); // wait until sending is possible
	UDR0 = c; // output character saved in c
}

void uart_puts(char *s){
	while(*s){
		uart_putc(*s);
		s++;
	}
}

void APDS9960_Init()										/* Gyro initialization function */
{
	_delay_ms(150);										/* Power up time >100ms */
	i2c_start(0x72);								/* Start with device write address */
	i2c_write(0X80);								/* Write to sample rate register */
	i2c_write(0x03);									/* Enable Power & ALS */
	i2c_stop();

	i2c_start(0x72);
	i2c_write(0X81);								/* Write to power management register */
	i2c_write(0XB6);									/*  */
	i2c_stop();

	i2c_start(0x72);
	i2c_write(0X8F);									/* Write to Configuration register */
	i2c_write(0X00);									/*   */
	i2c_stop();

}

void APDS9960_Start_Loc()
{
	i2c_start(0x72);								/* I2C start with device write address */
	i2c_write(APDS9960_CDATAL);							/* Write start location address from where to read */ 
	i2c_stop();

	i2c_start(0X73);
}

/*
	This function is used to initialize the ESP8266-01 module
	by setting up a connection with your WiFi.
*/
void Ready_ESP(){
	
	uart_puts(CIPMUX);
	uart_puts(CR_NL);
	uart_puts(CWJAP);
	uart_puts(SSID);
	uart_puts(Password);
	uart_puts(CR_NL);
	
	
	
	
}

/*This function is used to send the data acquired by the light sensor
  through the ESP8266-01 to ThingSpeak*/
void Send_to_ESP(){
	
	uart_puts(CIPSTART);
	uart_puts(CR_NL);
	uart_puts(CIPSEND);
	uart_puts(TS_COMMAND);
	uart_puts(API_KEY);
	uart_puts(FIELD2);
	uart_puts(FIELD3);
	uart_puts(FIELD4);
	uart_puts(CR_NL);
	uart_puts(CIPCLOSE);
	uart_puts(CR_NL);
	
}
	
	
//Reads the raw values from the sensor	
void APDS9960_Read_RawValue()
{
	APDS9960_Start_Loc();									/* Read Gyro values */
	ambient_light = (((uint8_t)i2c_read_ack()<<8) | i2c_read_ack());
	red_light = (((uint8_t)i2c_read_ack()<<8) | i2c_read_ack());
	green_light = (((uint8_t)i2c_read_ack()<<8) | i2c_read_ack());
	blue_light = (((uint8_t)i2c_read_ack()<<8) | i2c_read_ack());
	i2c_stop();
}

//Timer1 Initialization function
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

ISR(TIMER1_OVF_vect)
{
	//Keep track of number of overflows
	time_ovf++;
	
}

int main()
{
	char ambient_buffer[10];
	char R_buffer[10];
	char G_buffer[10];
	char B_buffer[10];
	i2c_init();											/* Initialize I2C */
	APDS9960_Init();										/* Initialize APDS9960 */
	USART_Init(9600);									/* Initialize USART with 9600 baud rate */
	uint8_t al = 0;
	uint8_t R = 0;
	uint8_t G = 0;
	uint8_t B = 0;
	time_ovf = 0;
	
	while(1)
	{
		APDS9960_Read_RawValue(); //Reads raw values
		
		itoa(ambient_light, ambient_buffer, 10); //Converts ambient light value
		uart_puts(ambient_buffer);				//Sends converted value to UART
		uart_puts("  ");
		
		itoa(R, R_buffer, 10);
		uart_puts(R_buffer);
		uart_puts("  ");
		
		itoa(G, G_buffer, 10);
		uart_puts(G_buffer);
		uart_puts("  ");
		
		itoa(B, B_buffer, 10);
		uart_puts(B_buffer);
		uart_puts("  ");
		uart_puts(CR_NL);
		
		if(time_ovf >=58){ //Timer condition specified for approximately 15 seconds to send data to ThingSpeak
			Send_to_ESP();
			time_ovf = 0;
		}

		else
			_delay_ms(20);

	}
}
