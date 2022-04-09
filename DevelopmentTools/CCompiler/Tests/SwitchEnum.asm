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
  isub SP, 1
  mov R0, 0
  mov [BP-1], R0
  mov R0, [BP-1]
  mov R1, 0
  ieq R1, R0
  jt R1, __switch_9_case_0
  mov R1, 1
  ieq R1, R0
  jt R1, __switch_9_case_1
  mov R1, 2
  ieq R1, R0
  jt R1, __switch_9_case_2
  jmp __switch_9_end
__switch_9_case_0:
  jmp __switch_9_end
__switch_9_case_1:
__switch_9_case_2:
__switch_9_end:
__function_main_return:
  mov SP, BP
  pop BP
  ret

