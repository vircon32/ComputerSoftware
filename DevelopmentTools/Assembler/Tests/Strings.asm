__main:
  mov R0, [__message]
  hlt
  
__message:
  string "hello"

__message_escaped:
  string "\"hello\""