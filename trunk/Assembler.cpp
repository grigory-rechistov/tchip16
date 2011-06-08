#include "Assembler.h"

void Assembler::writeOp(std::vector<std::string>& tokens) {
	output.put();
}

inline void Assembler::op_void(OPCODE op) {
	output.put(0x0);
	output.put(0x0);
	output.put(0x0);
}

inline void Assembler::op_imm(OPCODE op,u16 imm) {
	output.put(0x0);
	output.put(imm & 0x00FF);
	output.put(imm >> 8);
}