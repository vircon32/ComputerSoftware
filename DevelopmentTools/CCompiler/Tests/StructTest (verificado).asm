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
  
  ; 3* Point + 1* Pointer
  ; P is 2 words: &P = (BP-1)-1 = BP-2
  isub SP, 7
  
  ; P.x = 10;
  mov R0, 10
  mov [BP-2], R0    ; &P.x = &P+0 = BP-2
  
  ; P.y = -P.x;
  mov R0, [BP-2]
  isgn R0
  mov [BP-1], R0    ; &P.y = &P+1 = BP-1
  
  ; P1 = P2;
  lea R13, [BP-4]   ; DR = &P1 = BP-4
  lea R12, [BP-6]   ; SR = &P2 = BP-6
  mov CR, 2
  movs
  
  ; P1 == P2;
  lea R13, [BP-4]   ; DR = &P1 = BP-4
  lea R12, [BP-6]   ; SR = &P2 = BP-6
  mov CR, 2
  cmps R0
  bnot R0
  
  ; int PointedX = (&P)->x;
  lea R1, [BP-2]
  mov R0, [R1]
  mov [BP-7], R0
  
__function_main_return:
  mov SP, BP
  pop BP
  ret

