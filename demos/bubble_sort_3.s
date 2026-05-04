# 40 element bubble sort:
bubble_sort:
    LDI R2 9
    LDI R3 10
    LDI R4 4
    LDI R5 7
    LDI R6 8
    LDI R7 2
    LDI R8 5
    LDI R9 6
    LDI R10 3
    LDI R11 1
    LDI R0 80
    LDI R1 40
    STR R0 R2
    STRB R3 R0 1
    STRB R4 R0 2
    STRB R5 R0 3
    STRB R6 R0 4
    STRB R7 R0 5
    STRB R8 R0 6
    STRB R9 R0 7
    STRB R10 R0 8
    STRB R11 R0 9


    STRB R2 R0 10
    STRB R3 R0 11
    STRB R4 R0 12
    STRB R5 R0 13
    STRB R6 R0 14
    STRB R7 R0 15
    STRB R8 R0 16
    STRB R9 R0 17
    STRB R10 R0 18
    STRB R11 R0 19

    STRB R2 R0 20
    STRB R3 R0 21
    STRB R4 R0 22
    STRB R5 R0 23
    STRB R6 R0 24
    STRB R7 R0 25
    STRB R8 R0 26
    STRB R9 R0 27
    STRB R10 R0 28
    STRB R11 R0 29

    STRB R2 R0 30
    STRB R3 R0 31
    STRB R4 R0 32
    STRB R5 R0 33
    STRB R6 R0 34
    STRB R7 R0 35
    STRB R8 R0 36
    STRB R9 R0 37
    STRB R10 R0 38
    STRB R11 R0 39

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


# Add data directive
# 80 elements array
# 64 words in cache, 16 line cache

