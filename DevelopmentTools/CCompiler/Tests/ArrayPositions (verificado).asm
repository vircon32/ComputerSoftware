; program start section
  call __global_scope_initialization
  call __function_main
  hlt

; location of global variables
  define global_Bubbles 0

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
  
  ; int* B = &Bubbles[0][0];
  mov R0, global_Bubbles    ; R0 = &Bubbles[0][0]
  mov [BP-1], R0            ; B = &Bubbles[0][0]
  
  ; B[1] = 9;
  mov R0, 9         ; R0 = 9
  mov R1, [BP-1]    ; R1 = B
  iadd R1, 1        ; R1 = B+1
  mov [R1], R0      ; *(B+1) = 9
  
  ; *(B+2) = 11;
  mov R0, 11        ; R0 = 11
  mov R1, [BP-1]    ; R1 = B
  iadd R1, 2        ; R1 = B+2
  mov [R1], R0      ; *(B+2) = 11

  ; *(--B) = 13;
  mov R0, 13        ; R0 = 13
  mov R1, [BP-1]    ; R1 = B
  isub R1, 1        ; R1 = B-1
  mov [BP-1], R1    ; B = B-1
  mov R1, [R1]      ; R1 = *(B-1)
  lea R1, [BP-1]    ; R1 = BP-1 = &B
  mov R1, [R1]      ; R1 = B
  mov [R1], R0      ; *B = 13
  
  ; *(B++) = 15;
  mov R0, 15        ; R0 = 15
  mov R1, [BP-1]    ; R1 = B
  mov R2, R1        ; R2 = B
  iadd R2, 1        ; R1 = B+1
  mov [BP-1], R2    ; B = B+1
  mov [R1], R0      ; B = 15   (usa la B original)
  
  ; (B+=17) = &Bubbles[0][0];
  mov R0, global_Bubbles    ; R0 = &Bubbles[0][0]
  mov R1, [BP-1]            ; R1 = B
  iadd R1, 17               ; R1 = B+17
  mov [BP-1], R1            ; B = B+17
  mov [BP-1], R0            ; B = &Bubbles[0][0] 
  
__function_main_return:
  mov SP, BP
  pop BP
  ret

