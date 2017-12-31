#ifndef OPCODES_H
#define OPCODES_H

// NOTE: 00 is reserved for return status
#define OP_NOP      0x01
#define OP_JMP      0x02
#define OP_LOAD     0x03
#define OP_DPUSH    0x04
#define OP_RPUSH    0x05
#define OP_DPOP     0x06
#define OP_RPOP     0x07
#define OP_INC      0x08
#define OP_DEC      0x09
#define OP_HLT      0xFF

#define REG_PC      0x01
#define REG_IP      0x02
#define REG_CA      0x03
#define REG_X       0x10
#define REG_Y       0x11

unsigned char op_name_to_code(char *name);
char *op_code_to_name(unsigned char code);

#endif
