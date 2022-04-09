; location of global variables
  define global_MyString 1

; program start section
  call __global_scope_initialization
  call __function_main
  halt

__global_scope_initialization:
  push BP
  mov BP, SP
  mov R0, __literal_string_9
  mov [global_MyString], R0
  mov SP, BP
  pop BP
  ret

__function_ProcessString:
  push BP
  mov BP, SP
  mov R0, [BP+2]
__function_ProcessString_return:
  mov SP, BP
  pop BP
  ret

__function_main:
  push BP
  mov BP, SP
  isub SP, 3
  mov R1, [global_MyString]
  mov [SP], R1
  call __function_ProcessString
  mov [BP-1], R0
  mov R1, __literal_string_22
  mov [SP], R1
  call __function_ProcessString
  mov [BP-2], R0
  mov R0, __literal_string_23
__function_main_return:
  mov SP, BP
  pop BP
  ret

__literal_string_9:
  string "Hello"
__literal_string_22:
  string "Bye"
__literal_string_23:
  string "Unused\nString"
