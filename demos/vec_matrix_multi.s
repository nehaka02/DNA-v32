# Bench mark matrix multiply no vector
# Matrix 1 = [[1, 2, 3, 4],
#             [5, 6, 7, 8],
#             [1, 2, 3, 4],
#             [5, 6, 7, 8]]

# Matrix 2 = [[1, 1, 1, 1],
#             [-1, -1, -1, -1],
#             [1, 1, 1, 1],
#             [-1, -1, -1, -1]]

# Answer = [[-2, -2, -2, -2],
#           [-2, -2, -2, -2],
#           [-2, -2, -2, -2],
#           [-2, -2, -2, -2]]

# Load matrix 1 into memory 
ldi r0, 1
ldi r1, 2
ldi r2, 3
ldi r3, 4
ldi r4, 5
ldi r5, 6
ldi r6, 7
ldi r7, 8
# Make sure start address don't collide with instructions 
ldi r9, 80       # Starting address to store
str r9, r0
strb r1, r9, 1
strb r2, r9, 2
strb r3, r9, 3
strb r4, r9, 4
strb r5, r9, 5
strb r6, r9, 6
strb r7, r9, 7
strb r0, r9, 8
strb r1, r9, 9
strb r2, r9, 10
strb r3, r9, 11
strb r4, r9, 12
strb r5, r9, 13
strb r6, r9, 14
strb r7, r9, 15

# Load matrix 2 into memory 
ldi r1, -1
strb r0, r9, 16
strb r1, r9, 17
strb r0, r9, 18
strb r1, r9, 19
strb r0, r9, 20
strb r1, r9, 21
strb r0, r9, 22
strb r1, r9, 23
strb r0, r9, 24
strb r1, r9, 25
strb r0, r9, 26
strb r1, r9, 27
strb r0, r9, 28
strb r1, r9, 29
strb r0, r9, 30
strb r1, r9, 31

# End of matrix upload/set up (break point at = 45)

# Vectorized Matrix multiply
# A at address 80, BT at address 96, C at address 120
# r0 = base A, r1 = base BT, r2 = base C
# r3 = i, r4 = j
# r5 = addr of row i of A
# r6 = addr of row j of BT
# r7 = addr of C[i][j]
# r8 = result of VSUM (dot product)

ldi r0, 80         # base address of A
ldi r1, 96         # base address of BT
ldi r2, 120        # base address of C
ldi r3, 0          # i = 0

LOOP_I:                 # For each row in BT
    cmpi r3, 4
    bge END_I
    ldi r4, 0           # j = 0
    LOOP_J:             # For each row in A 
        cmpi r4, 4
        bge END_J

        # compute addr of row i of A = base_A + i*4
        muli r5, r3, 4      # r5 = i * 4
        add r5, r5, r0      # r5 = base_A + i*4

        # compute addr of row j of BT = base_BT + j*4
        muli r6, r4, 4      # r6 = j * 4
        add r6, r6, r1      # r6 = base_BT + j*4

        # load row i of A into q0
        vld q0, r5

        # load row j of BT into q1
        vld q1, r6

        # element wise multiply
        vmul 4, q2, q0, q1  # q2 = q0 * q1

        # sum all elements = dot product
        vsum 4, q2, r8      # r8 = sum of q2

        # store result in C[i][j] = base_C + i*4 + j
        muli r7, r3, 4      # r7 = i * 4
        add r7, r7, r2      # r7 = base_C + i*4
        add r7, r7, r4      # r7 = base_C + i*4 + j
        strb r8, r7, 0      # C[i][j] = dot product

        addi r4, r4, 1      # j++
        b LOOP_J

    END_J:
        addi r3, r3, 1      # i++
        b LOOP_I

END_I:
    halt
    halt
    halt
    halt