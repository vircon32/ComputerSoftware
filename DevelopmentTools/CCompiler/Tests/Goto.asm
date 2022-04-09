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
__label_2_start:
__if_3_start:
  mov R0, 1
  jf R0, __if_3_end
  jmp __label_8_end
  jmp __label_2_start
__if_3_end:
__label_8_end:
__function_main_return:
  mov SP, BP
  pop BP
  ret

