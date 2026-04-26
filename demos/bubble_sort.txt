bubble_sort:
    LDI R8 5
    LDI R9 6
    LDI R10 3
    LDI R0 32
    LDI R1 3
    STR R0 R8
    STRB R9 R0 1
    STRB R10 R0 2
    XORI R2 R1 0         # r2 = n

outer_loop:

    CMPI R2 1
    BLE done

    LDI R3 0            # i = 0
    SUBI R4 R2 1        # inner limit


inner_loop:

    CMP R3 R4
    BGE next_pass

    ADD R6 R0 R3           # r6 = &a[i] (word addressing, no scaling)

    LD R7 R6               # r7 = a[i]
    LDB R5 R6 1            # r5 = a[i+1]

    CMP R7 R5
    BLE no_swap

    STR R6 R5              # a[i] = a[i+1]
    STRB R7 R6 1           # a[i+1] = old a[i]


no_swap:

    ADDI r3, r3, 1
    B inner_loop

next_pass:
    SUBI r2, r2, 1
    B outer_loop

done:
    HALT
    HALT
    HALT
    HALT
    

