; program start section
  call __global_scope_initialization
  call __function_main
  hlt

; location of global variables

__global_scope_initialization:
  push BP
  mov BP, SP
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 2
  mov R0, 1
__switch_4_end:
  mov R0, [BP-1]
  mov R1, -7
  ieq R1, R0
  jt R1, __switch_6_case_minus_7
  mov R1, 1
  ieq R1, R0
  jt R1, __switch_6_case_1
  mov R1, 7
  ieq R1, R0
  jt R1, __switch_6_case_7
  jmp __switch_6_end
__switch_6_case_1:
  jmp __switch_6_end
__switch_6_case_7:
__switch_6_case_minus_7:
__switch_6_end:
__function_main_return:
  mov SP, BP
  pop BP
  ret

