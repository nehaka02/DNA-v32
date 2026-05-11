# Bench mark matrix multiply no vector
# Matrix 1 = [[1, 2, 3, 4, 5, 6, 7, 8],
#             [9, 10, 11, 12, 13, 14, 15, 16],
#             [1, 2, 3, 4, 5, 6, 7, 8],
#             [9, 10, 11, 12, 13, 14, 15, 16],
#             [1, 2, 3, 4, 5, 6, 7, 8],
#             [9, 10, 11, 12, 13, 14, 15, 16], 
#             [1, 2, 3, 4, 5, 6, 7, 8],
#             [9, 10, 11, 12, 13, 14, 15, 16]]

# Matrix 2 = [[1, 1, 1, 1, 1, 1, 1, 1],
#             [-1, -1, -1, -1, -1, -1, -1, -1],
#             [1, 1, 1, 1, 1, 1, 1, 1],
#             [-1, -1, -1, -1, -1, -1, -1, -1],
#             [1, 1, 1, 1, 1, 1, 1, 1],
#             [-1, -1, -1, -1, -1, -1, -1, -1]
#             [1, 1, 1, 1, 1, 1, 1, 1],
#             [-1, -1, -1, -1, -1, -1, -1, -1]]

# Answer = [[-4, -4, -4, -4, -4, -4, -4, -4],
#           [-4, -4, -4, -4, -4, -4, -4, -4],
#           [-4, -4, -4, -4, -4, -4, -4, -4],
#           [-4, -4, -4, -4, -4, -4, -4, -4],
#           [-4, -4, -4, -4, -4, -4, -4, -4],
#           [-4, -4, -4, -4, -4, -4, -4, -4],
#           [-4, -4, -4, -4, -4, -4, -4, -4],
#           [-4, -4, -4, -4, -4, -4, -4, -4]]



# End of matrix upload/set up (break point at = 45)

# Start matrix multiply 

# Matrix multiply setup
# Matrix 1(A) at address 80, matrix 2 transpose(BT) at address 100, solution at address 120
# r0 = base A, r1 = base BT, r2 = base C
# r3 = i, r4 = j, r5 = k, r6 = acc
# r7 = A[i][k], r8 = BT[j][k]
# r9 = addr A[i][k], r10 = addr BT[j][k], r11 = addr C[i][j]

ldi r0, 40          # base address of A
ldi r1, 112         # base address of BT
ldi r2, 184         # base address of C
ldi r3, 0           # i = 0

LOOP_I:
    cmpi r3, 8
    bge END_I
    ldi r4, 0       # j = 0

    LOOP_J:
        cmpi r4, 8
        bge END_J
        ldi r6, 0   # acc = 0
        ldi r5, 0   # k = 0

        LOOP_K:
            cmpi r5, 8
            bge END_K

            # compute addr of A[i][k] = base_A + i*8 + k
            muli r9, r3, 8      # r9 = i * 8 
            add r9, r9, r0      # r9 = base_A + i*8
            add r9, r9, r5      # r9 = base_A + i*8 + k
            ldb r7, r9, 0       # r7 = A[i][k]

            # compute addr of BT[j][k] = base_BT + j*8 + k
            muli r10, r4, 8     # r10 = j * 8 
            add r10, r10, r1    # r10 = base_BT + j*8
            add r10, r10, r5    # r10 = base_BT + j*8 + k
            ldb r8, r10, 0      # r8 = BT[j][k]

            # multiply and accumulate
            mul r12, r7, r8     # r12 = A[i][k] * BT[j][k]
            add r6, r6, r12     # acc += r12

            addi r5, r5, 1      # k++
            b LOOP_K

        END_K:
            # store C[i][j] = base_C + i*8 + j
            muli r11, r3, 8     # r11 = i * 8 
            add r11, r11, r2    # r11 = base_C + i*8
            add r11, r11, r4    # r11 = base_C + i*8 + j
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
    halt
    halt
    halt

# Add data directive
# 6 x 6 matrix multiply 

# Load matrix 1 into memory 
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
# Filler 
0
0
0
0
0
0
0
0
# Load matrix 2 into memory 
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1
1
-1