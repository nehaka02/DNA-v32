# This Python file uses the following encoding: utf-8
import sys
from mnemonic_table import mnemonic_table


class Assembler:

    def __init__(self):
        self.symbol_table = {}
        self.machine_code = []

    def first_pass(self, lines):
        # Build symbol table
        location = 0

        for line in lines:
            line = line.strip()
            # if empty line or comment, continue
            if not line or line.startswith("#"):
                continue

            line = line.split('#')[0].strip()
            # if line is a label add to symbol table (assume labels are structured as label:)
            # label is everything before : in the line
            if ":" in line:
                label = line.split(":")[0].strip()
                self.symbol_table[label] = location

            # otherwise, increment location counter
            else:
                location += 1

    # def second_pass(self, lines):
    #     location = 0
    #     for line in lines:
    #         line = line.strip()
    #         # if valid instruction and not a label
    #         if ":" not in line:
    #             tokens = line.split(" ")
    #             operation = tokens[0]
    #             type_code = mnemonic_table[operation][0]
    #             opcode = mnemonic_table[operation][1]
    #             instr_struct = mnemonic_table[operation][2]

    #             binary = 0
    #             if instr_struct == "scalar":
    #                 # dest(reg) src1(reg) src2(reg)
    #                 dest, src1, src2 = tokens[1], tokens[2], tokens[3]
    #                 binary = self.__encode_scalar(type_code, opcode, dest, src1, src2)

    #             elif instr_struct == "immediate":
    #                 # dest(reg) src1(reg) immediate
    #                 dest, src1, immediate = tokens[1], tokens[2], tokens[3]
    #                 binary = self.__encode_immediate(type_code, opcode, dest, src1, immediate)

    #             elif instr_struct == "vector":
    #                 # length(immediate) dest(reg) src1(reg) src2(reg)
    #                 length, dest, src1, src2 = tokens[1], tokens[2], tokens[3], tokens[4]
    #                 binary = self.__encode_vector(type_code, opcode, length, dest, src1, src2)

    #             elif instr_struct == "vector_misc":
    #                 length, reg1, reg2 = tokens[1], tokens[2], tokens[3]
    #                 binary = self.__encode_vector_misc(type_code, opcode, length, reg1, reg2)

    #             elif instr_struct == "cmp":
    #                 src1, src2 = tokens[1], tokens[2]
    #                 binary = self.__encode_cmp(type_code, opcode, src1, src2)

    #             elif instr_struct == "cmpi":
    #                 src1, immediate = tokens[1], tokens[2]
    #                 binary = self.__encode_cmpi(type_code, opcode, src1, immediate)

    #             elif instr_struct == "branch":
    #                 offset = tokens[1]
    #                 binary = self.__encode_branch(type_code, opcode, offset, location)

    #             elif instr_struct == "bx":
    #                 src = tokens[1]
    #                 binary = self.__encode_bx(type_code, opcode, src)

    #             elif instr_struct == "misc":
    #                 dest, src = tokens[1], tokens[2]
    #                 binary = self.__encode_misc(type_code, opcode, dest, src)

    #             elif instr_struct == "halt":
    #                 binary = self.__encode_halt(type_code, opcode)

    #             elif instr_struct == "base":
    #                 reg1, reg2, offset = tokens[1], tokens[2], tokens[3]
    #                 binary = self.__encode_base(type_code, opcode, reg1, reg2, offset)

    #             elif instr_struct == "ldi":
    #                 dest, immediate = tokens[1], tokens[2]
    #                 binary = self.__encode_ldi(type_code, opcode, dest, immediate)

    #             self.machine_code.append(binary)
    #             location += 1

    def second_pass(self, lines):
            location = 0
            for line in lines:
                line = line.strip()
                # Skip empty lines and comments (same as first_pass!)
                if not line or line.startswith("#"):
                    continue

                # Skip label lines
                if ":" in line:
                    continue

                # Remove comments on the same line as the instruction
                line = line.split('#')[0].strip()
                #tokens = line.split(), old, new can handle commas
                tokens = line.replace(',', ' ').split()
                # if valid instruction and not a label
                operation = tokens[0].upper()
                type_code = mnemonic_table[operation][0]
                opcode = mnemonic_table[operation][1]
                instr_struct = mnemonic_table[operation][2]

                binary = 0
                if instr_struct == "scalar":
                    # dest(reg) src1(reg) src2(reg)
                    dest, src1, src2 = tokens[1], tokens[2], tokens[3]
                    binary = self.__encode_scalar(type_code, opcode, dest, src1, src2)

                elif instr_struct == "immediate":
                    # dest(reg) src1(reg) immediate
                    dest, src1, immediate = tokens[1], tokens[2], tokens[3]
                    binary = self.__encode_immediate(type_code, opcode, dest, src1, immediate)

                elif instr_struct == "vector":
                    # length(immediate) dest(reg) src1(reg) src2(reg)
                    length, dest, src1, src2 = tokens[1], tokens[2], tokens[3], tokens[4]
                    binary = self.__encode_vector(type_code, opcode, length, dest, src1, src2)

                elif instr_struct == "vector_misc":
                    length, reg1, reg2 = tokens[1], tokens[2], tokens[3]
                    binary = self.__encode_vector_misc(type_code, opcode, length, reg1, reg2)

                elif instr_struct == "cmp":
                    src1, src2 = tokens[1], tokens[2]
                    binary = self.__encode_cmp(type_code, opcode, src1, src2)

                elif instr_struct == "cmpi":
                    src1, immediate = tokens[1], tokens[2]
                    binary = self.__encode_cmpi(type_code, opcode, src1, immediate)

                elif instr_struct == "branch":
                    offset = tokens[1]
                    binary = self.__encode_branch(type_code, opcode, offset, location)

                elif instr_struct == "bx":
                    src = tokens[1]
                    binary = self.__encode_bx(type_code, opcode, src)

                elif instr_struct == "misc":
                    dest, src = tokens[1], tokens[2]
                    binary = self.__encode_misc(type_code, opcode, dest, src)

                elif instr_struct == "halt":
                    binary = self.__encode_halt(type_code, opcode)

                elif instr_struct == "base":
                    reg1, reg2, offset = tokens[1], tokens[2], tokens[3]
                    binary = self.__encode_base(type_code, opcode, reg1, reg2, offset)

                elif instr_struct == "ldi":
                    dest, immediate = tokens[1], tokens[2]
                    binary = self.__encode_ldi(type_code, opcode, dest, immediate)

                self.machine_code.append(binary)
                location += 1



    def assemble(self, input_file, output_file):
        # Read input_file into lines
        with open(input_file, "r") as f:
            lines = f.readlines()

        # First pass to generate symbol table
        self.first_pass(lines)

        # Second pass to generate machine code, store in python list
        self.second_pass(lines)

        # Write binary output to output file
        # with open(output_file, "w") as f:
        #     for instr in self.machine_code:
        #         f.write(f"{instr}\n")

        with open(output_file, "w") as f:
            address = 0
            for instr in self.machine_code:
                # match your parseInput format: W addr value size
                f.write(f"W {address} {instr} 4\n")
                address += 1

    # helper methods
    def __reg_to_num(self, reg):
        return int(reg[1:])

    def __encode_scalar(self, type_code, opcode, dest, src1, src2):
        dest = self.__reg_to_num(dest)
        src1 = self.__reg_to_num(src1)
        src2 = self.__reg_to_num(src2)
        return (type_code << 30) | (opcode << 25) | (dest << 21) | (src1 << 17) | (src2 << 13)

    # Note: int (numStr, 0) allows auto detect so input can be 0x#### 0b###, or normal integer
    def __encode_immediate(self, type_code, opcode, dest, src1, immediate):
        # 17 bit immediate, 0x1FFFF is 17 1's
        dest = self.__reg_to_num(dest)
        src1 = self.__reg_to_num(src1)
        immediate = int(immediate, 0)
        return (type_code << 30) | (opcode << 25) | (dest << 21) | (src1 << 17) | (immediate & 0x1FFFF)


    def __encode_vector(self, type_code, opcode, v_len, dest, src1, src2):
        dest = self.__reg_to_num(dest)
        src1 = self.__reg_to_num(src1)
        src2 = self.__reg_to_num(src2)
        return (type_code << 30) | (opcode << 25) | ((v_len & 0x3) << 23) | (dest << 19) | (src1 << 15) | (src2 << 11)


    def __encode_vector_misc(self, type_code, opcode, v_len, reg1, reg2):
        v_len = int(v_len, 0)
        reg1 = self.__reg_to_num(reg1)
        reg2 = self.__reg_to_num(reg2)
        return (type_code << 30) | (opcode << 25) | ((v_len & 0x3) << 23) | (reg1 << 19) | (reg2 << 15)


    def __encode_cmp(self, type_code, opcode, src1, src2):
        src1 = self.__reg_to_num(src1)
        src2 = self.__reg_to_num(src2)
        return (type_code << 30) | (opcode << 25) | (src1 << 21) | (src2 << 17)


    def __encode_cmpi(self, type_code, opcode, src1, immediate):
        # 21 bit immediate, 0x1FFFFF is 21 1's
        src1 = self.__reg_to_num(src1)
        immediate = int(immediate, 0)
        return (type_code << 30) | (opcode << 25) | (src1 << 21) | (immediate & 0x1FFFFF)


    def __encode_branch(self, type_code, opcode, offset, location): # FIXME
        # 26 bit offset
        # Calculate offset if needed, else convert to binary store in machine code
        if offset in self.symbol_table:
            offset = self.symbol_table[offset] - location
        else:
            offset = int(offset, 0)
        return (type_code << 30) | (opcode << 26) | (offset & 0x3FFFFFF)


    def __encode_bx(self, type_code, opcode, src):
        src = self.__reg_to_num(src)
        return (type_code << 30) | (opcode << 26) | (src << 22)


    def __encode_misc(self, type_code, opcode, dest, src):
        dest = self.__reg_to_num(dest)
        src = self.__reg_to_num(src)
        return (type_code << 30) | (opcode << 26) | (dest << 22) | (src << 18)


    def __encode_halt(self, type_code, opcode):
        return (type_code << 30) | (opcode << 26)


    def __encode_base(self, type_code, opcode, reg1, reg2, offset):
        # 18 bit offset
        reg1 = self.__reg_to_num(reg1)
        reg2 = self.__reg_to_num(reg2)
        offset = int(offset, 0)
        return (type_code << 30) | (opcode << 26) | (reg1 << 22) | (reg2 << 18) | (offset & 0x3FFFF)


    def __encode_ldi(self, type_code, opcode, dest, immediate):
        # 22 bit immediate
        dest = self.__reg_to_num(dest)
        immediate = int(immediate, 0)
        return (type_code << 30) | (opcode << 26) | (dest << 22) | (immediate & 0x3FFFFF)


if __name__ == "__main__":
    # arguments will be provided via code input in UI
    input_file  = sys.argv[1]   # temp.s
    output_file = sys.argv[2]   # output file for binary

    assembler = Assembler()
    assembler.assemble(input_file, output_file)

