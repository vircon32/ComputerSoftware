  jmp __main
  ; -------------------------------
  
__draw_scene:

  ; clear the screen
  out GPU_ClearColor, 0x000070FF
  out GPU_Command, GPUCommand_ClearScreen
  ret
  
__check_button_a:
  ; - returns in R0 a boolean
  in R0, INP_GamepadButtonA
  igt R0, 0
  ret
  
__main:
  
  call __draw_scene
  wait
  call __check_button_a
  ieq R0, 0
  jf R0, __restart
  hlt
  
__restart:
  jmp __main