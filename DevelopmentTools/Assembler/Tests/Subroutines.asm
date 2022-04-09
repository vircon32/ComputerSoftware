  jmp __main
  ; -------------------------------
  
__draw_scene:

  ; clear the screen
  out GPUClearColor, 0x000070FF
  out GPUCommand, GPUCommand_ClearScreen
  ret
  
__check_button_a:
  ; - returns in R0 a boolean
  mov R1, 2
  in R0, INP_GamepadButtonA
  igt R0
  ret
  
__main:
  
  call __draw_scene
  wait
  call __check_button_a
  icmp R0, 0
  jne _restart
  hlt
  
__restart:
  jmp _main