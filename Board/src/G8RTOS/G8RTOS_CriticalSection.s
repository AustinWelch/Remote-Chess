; G8RTOS_CriticalSectionASM.s
; Holds all ASM functions needed for Critical Sections
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def G8RTOS_StartCriticalSection, G8RTOS_EndCriticalSection
	
	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section
	


SavedPRIMASK_ptr: .field 32

; Starts a critical section
; 	- Saves the state of the current PRIMASK (I-bit)
; 	- Disables interrupts
; -----Returns: The current PRIMASK State
G8RTOS_StartCriticalSection:
	.asmfunc

	MRS r0, PRIMASK		; Save PRIMASK to R0 
	ldr r1, SavedPRIMASK_ptr ; Load the address of where we're saving the PRIMASK to
	str r0, [r1] 		; Save the PRIMASK to memory
	CPSID I				; Disable Interrupts
	BX LR				; Return

	.endasmfunc

; Ends a critical Section
; 	- Restores the state of the PRIMASK given an input
; -----Param R0: PRIMASK State to update
G8RTOS_EndCriticalSection:
	.asmfunc
	
	CPSIE I 			; Enable interrupts
	ldr r1, SavedPRIMASK_ptr ; Load the address of where we're saving the PRIMASK to
	ldr r0, [r1]		; Load the PRIMASK from memory
	MSR PRIMASK, r0		; Save R0 (Param) to PRIMASK
	BX LR				; Return
	
	.endasmfunc