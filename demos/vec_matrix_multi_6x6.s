# Bench mark matrix multiply with vector instructions
# Matrix 1 = [[1, 2, 3, 4, 5, 6],
#             [7, 8, 9, 10, 11, 12],
#             [1, 2, 3, 4, 5, 6],
#             [7, 8, 9, 10, 11, 12],
#             [1, 2, 3, 4, 5, 6],
#             [7, 8, 9, 10, 11, 12]]

# Matrix 2 = [[1, 1, 1, 1, 1, 1],
#             [-1, -1, -1, -1, -1, -1],
#             [1, 1, 1, 1, 1, 1],
#             [-1, -1, -1, -1, -1, -1]
#             [1, 1, 1, 1, 1, 1],
#             [-1, -1, -1, -1, -1, -1]]

# Answer = [[-3, -3, -3, -3, -3, -3],
#           [-3, -3, -3, -3, -3, -3],
#           [-3, -3, -3, -3, -3, -3],
#           [-3, -3, -3, -3, -3, -3],
#           [-3, -3, -3, -3, -3, -3],
#           [-3, -3, -3, -3, -3, -3]]



# Vectorized Matrix multiply
# A at address 80, BT at address 96, C at address 120
# r0 = base A, r1 = base BT, r2 = base C
# r3 = i, r4 = j
# r5 = addr of row i of A
# r6 = addr of row j of BT
# r7 = addr of C[i][j]
# r8 = result of VSUM (dot product)

ldi r0, 36         # base address of A
ldi r1, 84         # base address of BT
ldi r2, 140        # base address of C
ldi r3, 0          # i = 0

LOOP_I:                 # For each row in BT
    cmpi r3, 6
    bge END_I
    ldi r4, 0           # j = 0
    LOOP_J:             # For each row in A 
        cmpi r4, 6
        bge END_J

        # compute addr of row i of A = base_A + i*8
        muli r5, r3, 8      # r5 = i * 8
        add r5, r5, r0      # r5 = base_A + i*8

        # compute addr of row j of BT = base_BT + j*8
        muli r6, r4, 8      # r6 = j * 8
        add r6, r6, r1      # r6 = base_BT + j*8


        # === First chunk: elements [0..3] ===

        # load row i of A into q0
        vld q0, r5

        # load row j of BT into q1
        vld q1, r6

        # element wise multiply
        vmul 4, q2, q0, q1  # q2 = q0 * q1

        # sum all elements = dot product
        vsum 4, q2, r8      # r8 = sum of q2



        # === Second chunk: elements [4..5] ===
        addi r5, r5, 4      # r5 = addr of A[i][4]
        addi r6, r6, 4      # r6 = addr of BT[j][4]
        vld q3, r5          # loads A[i][4], A[i][5], pad, pad, length=2 ignores pad
        vld q4, r6         # loads BT[j][4], BT[j][5], pad, pad
        vmul 2, q5, q3, q4
        vsum 2, q5, r11     # r11 = partial dot product last two elements



        # store result in C[i][j] = base_C + i*8 + j
        add r8, r8, r11     # r8 = full dot product

        muli r7, r3, 8      # r7 = i * 8
        add r7, r7, r2      # r7 = base_C + i*8
        add r7, r7, r4      # r7 = base_C + i*8 + j
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

# Add data directive
# 6 x 6 matrix multiply 

# Load matrix 1 into memory 
1
2
3
4
5
6
0
0
7
8
9
10
11
12
0
0
1
2
3
4
5
6
0
0
7
8
9
10
11
12
0
0
1
2
3
4
5
6
0
0
7
8
9
10
11
12
0
0

# Load matrix 2 into memory 
1
-1
1
-1
1
-1
0
0
1
-1
1
-1
1
-1
0
0
1
-1
1
-1
1
-1
0
0
1
-1
1
-1
1
-1
0
0
1
-1
1
-1
1
-1
0
0
1
-1
1
-1
1
-1
0
0
