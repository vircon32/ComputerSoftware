; define data values: literal integers
_literals_a:
    integer 1, -3, 0xA
    
; define data values: literal floats
_literals_b:
    float  7.0, -0.9, 1.1

; define data values: strings
__message:
  string "hola"

; we can also embed the content of a file
; and access its first word from the label
__data:
  datafile "DataFile.dat"

; some custom definitions
define reg_0 R0
define pi 3.1416
