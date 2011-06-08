/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <map>
#include <vector>
#include <fstream>

#include "Opcodes.h"

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef signed char		s8;
typedef signed short	s16;
typedef signed int		s32;

// Assembler class, does the hard work
class Assembler {
public:
	Assembler();
	~Assembler();

	bool openFile(char*);
	bool parseLine();

	void outputFile();

private:
	// Remove ',' '\t' etc.
	void cleanLine(const std::string&);
	// Import another asm file (INCLUDE)
	bool importInc(const std::string&);
	// Import a binary file (IMPORTBIN)
	bool importBin(const std::string&, int, int, const std::string&);

	void writeOp(std::vector<std::string>&);
	void op_void(OPCODE);				// nop, cls, vblnk, ret, snd0, pushall, popall
	void op_imm(OPCODE,u16);			// jmp, jmc, jmz, jx, call, cx, spr, snd[1-3],
	void op_n(OPCODE,u8);				// bgc
	void op_n_n(OPCODE,u8,u8);			// flip
	void op_r_imm(OPCODE,u8,u16);		// rnd, ldi, ldm, stm, addi, subi, muli, divi, cmpi, andi, tsti, ori, xori
	void op_r_n(OPCODE,u8,u8);			// shl, shr, sal, sar
	void op_r_r_imm(OPCODE,u8,u8,u16);	// drw, jme
	void op_r_r_r(OPCODE,u8,u8,u8);		// add, sub, mul, div, cmp, and, tst, or, xor
	void db(std::vector<unsigned char>&);

	std::map<char*,int> labels;
	std::map<char*,int> consts;
	// Source code buffer
	std::string input;
	// Output file stream
	std::ofstream output;
	
	int lineNb;
	int curAddress;
}

#endif