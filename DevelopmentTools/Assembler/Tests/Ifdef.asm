%define DEBUG

%ifndef DEBUG
  %error "DEBUG is not defined!"
%else
  %define LOADZERO  mov R0, 0   ; do NOT use parentheses like in C!!
%endif

_main:
  LOADZERO
  hlt