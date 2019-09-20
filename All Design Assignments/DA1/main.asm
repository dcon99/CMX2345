;
; DA1.asm
;
; Created: 9/14/2019 7:35:40 PM
; Author : bruce

; Description: A small assembly program that multiplies through repeated addition using a
; 2-Byte multiplicand, a 2-Byte Multiplier and a 4-byte storage for the product.

ldi r25, 0b11001000 ; Initializes the register w/ Multiplicand's Lower Byte of the number 456
ldi r24, 0b00000001 ; Initializes the register w/ Multiplicand's Higher Byte of the number 456

ldi r23, 0b01001000 ; Initializes the register w/ Multiplier's Low Byte of the number 72
ldi r22, 0b00000000 ; Initializes the register w/ Multiplier's High Byte of the number 72

ldi r20, 0 ; Initializes for Product Low byte 1
ldi r19, 0 ; Initializes for Product High Byte 1
ldi r18, 0 ; Initializes for Product Low Byte 2
ldi r17, 0 ; Initializes for Product High Byte 2

ldi r16, 0 ; Register used to hold zero value for adc instructions.


start:				 ; Main function of this code. Repeatedly adds to multiply until the multiplier decrements to zero.
	add r20, r25	 ; Adds the lower byte of the multiplicand into r20
	adc r19, r24	 ; Adds the higher byte of the multiplicand into r19 w/ carry
	adc r18, r16	 ; Adds zero to register r18 with a carry
	adc r17, r16	 ; Adds zero to register r17 with a carry
	
	call decrement	 ; Calls the decrement function to reduce a two byte number by one.
	jmp start		 ; jumps back to start 

decrement:			; Function used to decrement a 2-Byte number used as the Multiplier.
	dec r23			; Decrements the lower byte of the multiplier.
	cpi r23, 0		; Compares the contents of R23 with the number 0.
	breq end		; Branches to the End function if R23 equals 0.
	sbci r22, 0		; Subtracts the higher byte of the multiplier with an immediate of 0 and the carry.
	ret				; Returns to Start function where it was called.


end:				; Ending function that keeps the code in an infinite loop of nop.
	nop				; No op instruction.
	jmp end			; Jumps back to the beginning of the End function.