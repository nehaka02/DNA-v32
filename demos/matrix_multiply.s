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

# Start matrix multiply 

# Matrix multiply setup
# Matrix 1(A) at address 80, matrix 2 transpose(BT) at address 100, solution at address 120
# r0 = base A, r1 = base BT, r2 = base C
# r3 = i, r4 = j, r5 = k, r6 = acc
# r7 = A[i][k], r8 = BT[j][k]
# r9 = addr A[i][k], r10 = addr BT[j][k], r11 = addr C[i][j]

ldi r0, 80          # base address of A
ldi r1, 96          # base address of BT
ldi r2, 120         # base address of C
ldi r3, 0           # i = 0

LOOP_I:
    cmpi r3, 4
    bge END_I
    ldi r4, 0       # j = 0

    LOOP_J:
        cmpi r4, 4
        bge END_J
        ldi r6, 0   # acc = 0
        ldi r5, 0   # k = 0

        LOOP_K:
            cmpi r5, 4
            bge END_K

            # compute addr of A[i][k] = base_A + i*4 + k
            muli r9, r3, 4      # r9 = i * 4
            add r9, r9, r0      # r9 = base_A + i*4
            add r9, r9, r5      # r9 = base_A + i*4 + k
            ldb r7, r9, 0       # r7 = A[i][k]

            # compute addr of BT[j][k] = base_BT + j*4 + k
            muli r10, r4, 4     # r10 = j * 4
            add r10, r10, r1    # r10 = base_BT + j*4
            add r10, r10, r5    # r10 = base_BT + j*4 + k
            ldb r8, r10, 0      # r8 = BT[j][k]

            # multiply and accumulate
            mul r12, r7, r8     # r12 = A[i][k] * BT[j][k]
            add r6, r6, r12     # acc += r12

            addi r5, r5, 1      # k++
            b LOOP_K

        END_K:
            # store C[i][j] = base_C + i*4 + j
            muli r11, r3, 4     # r11 = i * 4
            add r11, r11, r2    # r11 = base_C + i*4
            add r11, r11, r4    # r11 = base_C + i*4 + j
            strb r6, r11, 0     # C[i][j] = acc

            addi r4, r4, 1      # j++
            b LOOP_J

    END_J:
        addi r3, r3, 1          # i++
        b LOOP_I

END_I:
    halt
    halt
    halt
    halt

# 6 x 6 or 8 x 8


