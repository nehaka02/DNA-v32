# This Python file uses the following encoding: utf-8

# structure for the table entries:
# instruction name: (type code, opcode, instruction structure)

# instruction structure can be ...
    # scalar - for register register operaction (has dest)
    # immediate - for register immediate operation (has dest)
    # vector - normal vector operation (has dest)
    # vector_misc - operation length(immediate) vec_reg vec_reg
    # branch - normal branch operation PC relative
    # bx - register indirect branch
    # cmp - compare (no dest)
    # cmpi - compare immediate (no dest)
    # misc for miscellaneous (dest src)
    # halt
    # base (base + offset addressing mode)
    # ldi - load immediate value into register

mnemonic_table = {
# ALU data processing
    # dest(register) register register
    "ADD"  : (0, 0b00000, "scalar"),
    "SUB"  : (0, 0b00001, "scalar"),
    "DIV"  : (0, 0b00010, "scalar"),
    "MUL"  : (0, 0b00011, "scalar"),
    "MOD"  : (0, 0b00100, "scalar"),
    "ASR"  : (0, 0b00101, "scalar"),
    "ASL"  : (0, 0b00110, "scalar"),
    "LSR"  : (0, 0b00111, "scalar"),
    "LSL"  : (0, 0b01000, "scalar"),
    "AND"  : (0, 0b01001, "scalar"),
    "OR"   : (0, 0b01010, "scalar"),
    "XOR"  : (0, 0b01011, "scalar"),

    # length(immediate) dest(vector reg) vector_reg_1 vector_reg_2
    "VADD" : (0, 0b01100, "vector"),
    "VSUB" : (0, 0b01101, "vector"),
    "VMUL" : (0, 0b01110, "vector"),

    # dest(register) register immediate
    "ADDI" : (0, 0b01111, "immediate"),
    "SUBI" : (0, 0b10000, "immediate"),
    "MULI" : (0, 0b10001, "immediate"),
    "ASRI" : (0, 0b10010, "immediate"),
    "ASLI" : (0, 0b10011, "immediate"),
    "ANDI" : (0, 0b10100, "immediate"),
    "ORI"  : (0, 0b10101, "immediate"),
    "XORI" : (0, 0b10110, "immediate"),
    "DIVI" : (0, 0b10111, "immediate"),
    "MODI" : (0, 0b11000, "immediate"),
    "LSRI" : (0, 0b11001, "immediate"),
    "LSLI" : (0, 0b11010, "immediate"),

    # src1(register) src2(register)
    "CMP"  : (0, 0b11011, "cmp"),
    # length(immediate) vector_reg_1 vector_reg_2
    "VEQ"  : (0, 0b11100, "vector_misc"),
    "VSUM" : (0, 0b11101, "vector_misc"),
    # src1(register), immediate
    "CMPI" : (0, 0b11110, "cmpi"),

# Branch
    # branch offset(immediate, if symbol calculate the offset via assembler)
    "B"    : (1, 0b0000, "branch"),
    "BEQ"  : (1, 0b0001, "branch"),
    "BNE"  : (1, 0b0010, "branch"),
    "BLT"   : (1, 0b0011, "branch"), # less than
    "BLE"  : (1, 0b0100, "branch"),
    "BGT"   : (1, 0b0101, "branch"), # greater than
    "BGE"  : (1, 0b0110, "branch"),
    "BL"   : (1, 0b0111, "branch"),

    # branch src (branch to address stored in src register)
    "BX"   : (1, 0b1000, "bx"),

# Misc
    # dest(register) src(register)
    "NOT"  : (2, 0b0000, "misc"),
    "LD"   : (2, 0b0001, "misc"),
    "STR"  : (2, 0b0010, "misc"),
    "VLD"  : (2, 0b0011, "misc"),
    "VSTR" : (2, 0b0100, "misc"),

    # type code, opcode, left over
    "HALT" : (2, 0b0101, "halt"),

    # register, base(register), offset(immediate)
    "LDB"  : (2, 0b0110, "base"),
    "STRB" : (2, 0b0111, "base"),

    # dest_register immediate
    "LDI"  : (2, 0b1000, "ldi"),
}
