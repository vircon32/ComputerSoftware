; just wait 1 frame
_function_wait:
  wait
  ret

%define CALL_WAIT call _function_wait