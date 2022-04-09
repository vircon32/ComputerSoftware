; location of global variables

; program start section
  call __global_scope_initialization
  call __function_main
  halt

__global_scope_initialization:
  push BP
  mov BP, SP
  mov SP, BP
  pop BP
  ret

__function_Factorial:
  push BP
  mov BP, SP
  isub SP, 1
__if_start_3:
  mov R0, [BP+2]
  ile R0, 1
  jf R0, __if_end_3
  mov R0, 1
  jmp __function_Factorial_return
__if_end_3:
  mov R1, [BP+2]
  isub R1, 1
  mov [SP], R1
  call __function_Factorial
__function_Factorial_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 1
  mov R1, 3
  mov [SP], R1
  call __function_Factorial
__function_main_return:
  mov SP, BP
  pop BP
  ret

