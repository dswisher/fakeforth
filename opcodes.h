#ifndef OPCODES_H
#define OPCODES_H

// Opcodes - most-significant 6 bits
// NOTE: 00 is reserved for return status
#define OP_NOP      0x04
#define OP_JMP      0x08
#define OP_DPUSH    0x0C
#define OP_RPUSH    0x10
#define OP_DPOP     0x14
#define OP_RPOP     0x18
#define OP_INC      0x1C
#define OP_DEC      0x20
#define OP_LOAD     0x24
#define OP_STORE    0x28    // TODO - new - implement this!

#define OP_GO       0xF0    // TODO - hack until JMP modes done

#define OP_HLT      0xFC

// Address modes - least-significant two bits
#define ADDR_MODE0  0x00
#define ADDR_MODE1  0x01
#define ADDR_MODE2  0x02
#define ADDR_MODE3  0x03

// Registers
#define REG_PC      0x01
#define REG_IP      0x02
#define REG_CA      0x03
#define REG_X       0x10
#define REG_Y       0x11

unsigned char op_name_to_code(char *name);
char *op_code_to_name(unsigned char code);

#endif
