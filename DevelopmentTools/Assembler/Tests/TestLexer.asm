; data section for literal integers
_literals_a:
    integer 1, -3, 0xA, 'H'
    
; data section for literal Vector "b" in main
_literals_b:
    float  7.0, -0.9, 1.1

; some test definition
%define reg_0 R0
%define reg_1 R1

_sum:

  ; enter context
  push R6;BP
  mov BP, SP
  
  ; no local allocation needed!
  ; (do nothing)
  
  ; copy both operands from stack to registers
  ; they are in CALLER's stack frame! So index them using BP and not SP
  mov reg_0, [BP+2]
  mov	reg_1, [BP+1]
  
  ; perform calculation
  ; (R0 is used to store returned value)
  iadd reg_0, reg_1
  
  ; leave context
  pop BP
  ret

_main:

  ; entering context is not needed for main!!
  ; (do nothing)
  
  ; allocate stack space for locals
  ; (in this case, for the function call's 2 parameters)
  isub SP, 2
  
  ; place literal operands in the locals stack frame
  ; (but not using push!)
  mov R0, 1
  mov [SP+1], R0
  mov R0, 27
  mov [SP+2], R0
  
  ; call function
  call _sum
  
  ; place resulting value somewhere
  mov [BP-1], R0
  
  ; leaving context is not possible for main!
  hlt
