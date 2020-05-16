; convert a null-terminated character string to uppter case

		.ORIG x3000

						; get read start address
		LEA R0, RSTART			; load address of instruction rstart
		LDW R0, R0, #0			; load x4000
		LDW R0, R0, #0			; load mem[x4000] (char *)

						; get write start address
		LEA R1, WSTART			; load address of instruction wstart
		LDW R1, R1, #0			; load x4002
		LDW R1, R1, #0			; load mem[x4002] (char *)

						; converstion start
LOOP	LDB R2, R0, #0			; load character
		BRz EXIT				; if character is null, exit
		ADD R2, R2, #-16
		ADD R2, R2, #-16		; r2 - 32
WRITE	STB R2, R1, #0			; write r2 to address r1
		ADD R0, R0, #1			; move to next read address
		ADD R1, R1, #1			; move to next write address
		BR LOOP

EXIT	STB	R2, R1, #0			; store null-terminated character
		HALT

RSTART	.FILL x4000
WSTART	.FILL x4002

		.END
