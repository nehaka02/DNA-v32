bubble_sort:
LDI R8 5
LDI R9 6
LDI R10 1
LDI R11 3
STR R0 R8
STRB R0 R9 1
STRB R0 R10 2
STRB R0 R11 3
XORI R2 R1 0
outer_loop:
CMPI R2 1
BLE done
LDI R3 0
SUBI R4 R2 1
inner_loop:
CMP R3 R4
BGE next_pass
ADD R6 R0 R3
LD R7 R6
LDB R5 R6 1
CMP R7 R5
BLE no_swap
STR R6 R5
STRB R7 R6 1
no_swap:
ADDI R3 R3 1
B inner_loop
next_pass:
SUBI R2 R2 1
B outer_loop
done:
HALT
HALT
HALT
HALT


