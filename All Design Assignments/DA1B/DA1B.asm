;
; DA1B.asm
;
; Created: 9/21/2019 1:23:47 PM
; Author : bruce
; Description: This program's main objective is divided into 3 tasks:
; - The first is to take 250 numbers (1 to 250 respectively) and store each one into different memory addresses starting from 0x0100
; - The second is to parse through the stored list of numbers and check if they are divisible by 5.
;		- If they are divisible by 5 then they will be stored into a list starting from memory address 0x0300
;		- If they are not divisible by 5 then they will be placed into a list starting from memory address 0x0500
; - The third task is to parse through both lists and add up each number within those lists into 4 registers that form 2 16-bit registers to hold their respective sums

ldi XH, high(0x0300) ; Loads XH with high byte of memory address 0x0300
ldi XL, low(0x0300)  ; Loads XL with lower byte of memory address 0x0300

ldi YH, high(0x0500) ; Loads YH with the high byte of memory address 0x0500
ldi YL, low(0x0500)  ; Loads YL with the lower byte of memory address 0x0500

ldi r20, 200	; Place holder for comparing numbers above 200
ldi r21, 0		; Dividend/Remainder
ldi r22, 5		; Divisor
ldi r23, 1		; Register will hold the numbers loaded from memory address 0x0100
ldi r24, 250	; Holds the last number expected in the sequence to compare R23 with


ldi ZH, high(0x0100)	; Loads ZH with the high byte of memory address 0x0100
ldi ZL, low(0x0100)		; Loads ZL with the lower byte of memory address 0x0100


fill_numbers:			; This function is meant to fill in 250 memory addressess sequentially with 250 numbers
	st Z+, r23			; Stores R23 into address pointed at by register Z
	cpi r23, 250		; Compares r23 with the last number in the sequence 250
	breq reset_reg23	; If r23 equals 250 it branches to the reset_reg function
	inc r23				; Increments r23 by one
	jmp fill_numbers	; Jumps back to the beginning of this function



reset_reg23:					; This function simply clears the contents of r23 and loads in the first value of memory address 0x0100 into r23 while incrementing the Z register by one
	clr r23						; Clears the contents of r23
	ldi r23, 0					; Loads the immediate value of zero into r23
	ldi ZH, high(0x0100)		; Loads ZH with the high byte of memory address 0x0100
	ldi ZL, low(0x0100)			; Loads ZL with the lower byte of memory address 0x0100
	ld r23, Z+					; Loads r23 with the value in memory address 0x0100

main:							; This function is the main function of the program which will call other functions and repeat until all numbers from 1 to 250 have been sorted into their according memory addresses
	call load_reg21				; Calls the function load_reg21 which clears r21's contents and adds the value of r23 into r21
	call divide_by_5			; Calls the function divide_by_5 which checks if the current number is divisible by 5
	call check_if_zero			; Calls the function check_if_zero which checks if the remainder of the function divide_by_5 is 0, if true then it will store the number into register X which starts at address 0x0300
	call check_if_not_zero		; Calls the function check_if_not_zero which checks if the remainder of function divide_by_zero is not 0, if true then it will store thenumber into register Y which starts at address 0x0500
	call increment_Z			; Calls the mincrememnt_Z function which is meant to increment the Z pointer which parses through the first list

	cpi r23, 0					; Compares r23 with the number 0
	brne bk_to_main				; If r23 is not zero then it branches to the bk_to_main function
	ldi XH, high(0x0300)		; Loads XH with high byte of memory address 0x0300
	ldi XL, low(0x0300)			; Loads XL with lower byte of memory address 0x0300
	ldi YH, high(0x0500)		; Loads YH with the high byte of memory address 0x0500
	ldi YL, low(0x0500)			; Loads YL with the lower byte of memory address 0x0500

	jmp adding_X				; Jumps to the adding_X function
		

bk_to_main:						; This function is meant to jump back to the main function
	jmp main					; Jumps back to main

load_reg21:						; This function is meant to clear the contents of r21 and load it in with a new value the was stored into r23 from the Z register in order to use as remainder in divide_by_5 function
	clr r21						; Clears the contents of r21
	add r21, r23				; Adds the value of r23 into r21
	ret							; Returns to main

divide_by_5:					; This function is meant take the number loaded into r21 from r23 and divide through repeated subtraction by 5 until it is less than 5
	
	cpi  r23, 5					; Compares r23 with in order to skip numbers 1-4 and immediately place them into register Y as they are not divisible by 5 saving a few cycles in the process
	brlo return					; Branches to return if the number is less than 5 unsigned for numbers under 4
	cp   r21, r20				; Compares r21 with r20 to check if the number stored into r21 is higher than 200 in order to properly divide it as numbers greater than or equal to 200 are seen as negative numbers
	sub  r21, r22				; Subtracts r22 from r21
	cp r21, r22					; Compares r21 with r22 to see if it is less than 5
	brlo return					; Branches to the return function if the number is less than r22
	jmp divide_by_5				; Jumps back to the start of divide_by_5


	
check_if_zero:					; This function checks if the remainder from divide_by_5 stored in r21 is zero. If true, then it jumps to the load_num_X function, else it returns to main
	cpi r21, 0					; Compares r21 with the immediate of 0
	breq load_num_X				; Branches to the function _load_num_X if r21 equals 0
	ret							; Returns to main
	
increment_Z:					; This function is used to parse through the first list of numbers (1-250) using the Z register
	ld r23, Z+					; Loads the number the Z register is currently pointing to into r23 then increments the Z register
	ret							; Returns to the main function

load_num_X:						; This function is used to load the value in r23 should the value be divisible by 5 and stored into the list pointed to by the X register
	st X+, r23					; Stores the value held in r23 into the address pointed to by the X register and then incremented	
	ret							; Returns to the main function
	
return:							; A function used to return back to main
	ret							; Returns to main

check_if_not_zero:				; This function checks if the value in r21 is higher than zero. If true, then the function jumps to the function load_num_Y, else it returns to main
	cpi r21, 0					; Compares the value in r21 with the immediate zero
	brne load_num_Y				; Branches to the function load_num_Y if it is not zero
	ret							; Returns to main
	
load_num_Y:						; A function that stores the value held in r21 into the memory address pointed to by register Y
	st Y+, r23					; Stores the value in r23 into the memory pointed at by register Y
	ret							; Returns to main

adding_X:						; A function used to accomplish the third task of parsing through the list pointed to by the X register (which holds are numbers divisible by 5) and sums them accordingly into r17:r16 respectively
	clr r23						; Clears the contents of r23
	ld r23, X+					; Loads the value in the X register into r23 and points to the next memory address in X
	ld r1, -X					; Loads the value of X into r1 in order to decrement back to the previous address to not skip a number in the list
	add r16, r23				; Adds the value of r23 into the lower byte of r17:r16 16-bit register
	adc r17, r3					; Adds the value in r3 (which is 0) into the higher byte of r17:r16 16-bit register with the carry
	ld r1, X+					; Loads a value from X into r1 in order to move on to the next number
	clr r1						; Clears the contents of r1
	clc							; Clears the carry flag
	cpi r23, 0					; Compares r23 with 0 in order to see if the end of the list has been reached (value after the last number in the list will be zero)
	breq adding_Y				; Branches to adding_Y if it is zero
	jmp adding_X				; Jumps back to the start of this function

adding_Y:						; This function is used to parse through the list of numbers not divisible by 5 and add them into the 16-bit register combo of r19:r18 respectively
	clr r24						; Clears the contents of r24
	ld r24, Y+					; Loads the value pointed at by Y into r24 then points to the next address
	ld r2, -Y					; Loads a value into r2 in order to decrement Y to the previous address and not skip a number
	add r18, r24				; Adds the value in r24 to the lower byte of r19:r18
	adc r19, r3					; Adds the value of r3 (0) to the higher byte of r19:r18 with the carry
	ld r2, Y+					; Loads the value pointed at by Y into r2 to move to the next number in the list
	clr r2						; Clears the contents of r2
	clc							; Clears the carry flag
	cpi r24, 0					; Compares r24 with 0 as the number after the last value in the list is 0, indicating the end of the list
	breq end					; Branches to the end function of the program if r24 is 0
	jmp adding_Y				; Jumps back to the start of this function

end:							; The last function of the program that keeps it in an infinite loop by constantly jumping back to this function
	nop							; No operation done
	jmp end						; Jumps back to end