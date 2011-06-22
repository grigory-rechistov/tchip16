/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H

#include <map>
#include <vector>
#include <string>

#include "Error.h"
#include "Opcodes.h"

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef signed char		s8;
typedef signed short	s16;
typedef signed int		s32;

typedef std::vector<std::string> line;
typedef std::vector<line>		 lineList;

const u32 MEM_SIZE = 64*1024;

// Assembler class, does the hard work
class Assembler {
public:
	Assembler();
	~Assembler();

	void setOutputFile(const char*);
	// Build token array
	void tokenize(const char*);
	// Write buffer to disk
	void outputFile();
	// Command line modifier methods
	void useZeroFill();
	void useAlign();
	void useCaseSens();
	void useObsolete();
	void putMmap();
	// Debug use
	void debugOut();

private:
	// Import a binary file (IMPORTBIN)
	bool importBin(const std::string&, int, int, const std::string&);

	void writeOp(line&);
	void op_void(OPCODE);				// nop, cls, vblnk, ret, snd0, pushall, popall
	void op_imm(OPCODE,u16);			// jmp, jmc, jmz, jx, call, cx, spr, snd[1-3],
	void op_n(OPCODE,u8);				// bgc
	void op_n_n(OPCODE,u8,u8);			// flip
	void op_r_imm(OPCODE,u8,u16);		// rnd, ldi, ldm, stm, addi, subi, muli, divi, cmpi, andi, tsti, ori, xori
	void op_r_n(OPCODE,u8,u8);			// shl, shr, sal, sar
	void op_r_r_imm(OPCODE,u8,u8,u16);	// drw, jme
	void op_r_r_r(OPCODE,u8,u8,u8);		// add, sub, mul, div, cmp, and, tst, or, xor

	void db(std::vector<u8>&);
	void db(std::string&);
	// Adapted from prev. ver., useful str->int conversion
	u16 atoi_t(std::string&);

	// Parsed source file
	lineList tokens;
	// Lookup tables
	std::map<std::string,int> labels;
	std::map<std::string,int> consts;
	// Output byte buffer
	u8* buffer;
	// Output filename
	std::string outputFP;
	// Keep track of progress
	std::vector<std::string> filesImp;	// imported files (avoid cycles)
	int lineNb;							// line nb in tokens array
	int curAddress;						// address in output bin
	int totalBytes;						// size in B of output bin
	// Command line modifiers
	bool zeroFill;
	bool alignLabels;
	bool caseSens;
	bool allowObs;
	bool writeMmap;
};

#endif
