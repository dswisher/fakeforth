#ifndef OPCODES_H
#define OPCODES_H

// NOTE: 00 is reserved for return status
#define OP_NOP  0x01
#define OP_JMP  0x02
#define OP_LOAD 0x03
#define OP_HLT  0xFF

#define REG_IP  0x01

unsigned char op_name_to_code(char *opcode);

#endif
