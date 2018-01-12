#ifndef OPCODES_H
#define OPCODES_H

#define OPCODE(x)   (x << 2)

// Opcodes - most-significant 6 bits
// NOTE: 00 is reserved for return status
#define OP_NOP      OPCODE(1)
#define OP_JMP      OPCODE(2)
#define OP_DPUSH    OPCODE(3)
#define OP_RPUSH    OPCODE(4)
#define OP_DPOP     OPCODE(5)
#define OP_RPOP     OPCODE(6)
#define OP_INC      OPCODE(7)
#define OP_DEC      OPCODE(8)
#define OP_LOAD     OPCODE(9)
#define OP_STORE    OPCODE(10)
#define OP_GETC     OPCODE(11)
#define OP_PUTC     OPCODE(12)
#define OP_ADD      OPCODE(13)
#define OP_CALL     OPCODE(14)
#define OP_RET      OPCODE(15)
#define OP_CMP      OPCODE(16)
#define OP_JEQ      OPCODE(17)
#define OP_JNE      OPCODE(18)
#define OP_JGT      OPCODE(19)
#define OP_JLT      OPCODE(20)
#define OP_JGE      OPCODE(21)
#define OP_JLE      OPCODE(22)
#define OP_STOS     OPCODE(23)
#define OP_SUB      OPCODE(24)

#define OP_HLT      OPCODE(63)

// Address modes - least-significant two bits
#define ADDR_MODE0  0x00
#define ADDR_MODE1  0x01
#define ADDR_MODE2  0x02
#define ADDR_MODE3  0x03

// Registers
#define REG_PC      0x01
#define REG_IP      0x02
#define REG_CA      0x03
#define REG_I       0x0C
#define REG_J       0x0D
#define REG_M       0x0E
#define REG_N       0x0F
#define REG_X       0x10
#define REG_Y       0x11
#define REG_Z       0x12

// Bits for the flag register
#define FLAG_EQUAL  0x01
#define FLAG_GT     0x02    // greater than
#define FLAG_LT     0x04    // less than

unsigned char op_name_to_code(char *name);
char *op_code_to_name(unsigned char code);

#endif
