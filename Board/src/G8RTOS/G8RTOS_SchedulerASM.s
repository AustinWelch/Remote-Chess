; G8RTOS_SchedulerASM.s
; Holds all ASM functions needed for the scheduler
; Note: If you have an h file, do not have a C file and an S file of the same name

	; Functions Defined
	.def G8RTOS_StartFirstThread, PendSV_Handler

	; Dependencies
	.ref currentThread, G8RTOS_Scheduler, G8RTOS_StoreExcReturn, G8RTOS_GetExcReturn

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section


; Need to have the address defined in file 
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field currentThread, 32

; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack (r4-r11)
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:
	.asmfunc
	push {r4-r11} ; Push the remaining registers onto the stack. (The ISR into here pushed the rest)

	; mov r0, LR
	; bl G8RTOS_StoreExcReturn

	ldr r4, RunningPtr   ; Load the address of the current thread pointer (TCB_t**)
	ldr r5, [r4]		 ; Load the address of current thread's TCB into rt (dereference the TCB_t** above)
	str SP, [r5]		 ; Store the stack pointer into the TCB  (0 offset) (dereference the TCB_t*)
	; mrs r6, PSP			 ; Get the process stack pointer

	bl G8RTOS_Scheduler  ; Call the scheduler to set the next thread

	ldr r5, [r4]		 ; Load the address of the updated-current thread's TCB into r5
	ldr SP, [r5]		 ; Get the saved stack pointer of the updated-current thread
	; msr PSP, r6			 ; Restore the process stack pointer

    ; ldr r4, SHCSR
    ; and r4, ~(1 << 11) ; Clear the systick active bit
    ; bl G8RTOS_GetExcReturn
    ; mov LR, r0


	pop {r4-r11} ; Pop the previously pushed registers of the new thread
	mov LR, #0xFFF9
	movt LR, #0xFFFF
	bx LR
	.endasmfunc
	

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_StartFirstThread:
	.asmfunc
	; Don't care about the stack because we're never coming back

	ldr r4, RunningPtr   ; Load the address of the current thread pointer (TCB_t**)
	ldr r5, [r4]		 ; Load the address of current thread's TCB into rt (dereference the TCB_t** above)

	ldr SP, [r5]		 ; Get the saved stack pointer of the updated-current thread
	; msr PSP, r6			 ; Restore the process stack pointer

	; ldr r4, [r6, #-4]
	ldr LR, [SP, #56]	 ; Get the PC from the stack and put it into the return address (the entry point)
	bx LR				 ; Branch to the entry point by returning
	.endasmfunc

    .align
    .end


