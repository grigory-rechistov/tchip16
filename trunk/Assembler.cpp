/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>

#include "Assembler.h"

Assembler::Assembler() {
	buffer = new u8[MEM_SIZE]();
	lineNb = 0;
	curAddress = 0;
	zeroFill = false;
	alignLabels = false;
	caseSens = false;
	allowObs = false;
	writeMmap = false;
}

Assembler::~Assembler() {

}

bool Assembler::openFile(const char* fn) {
	std::ifstream in(fn);
	if(in.is_open()) {
		std::stringstream file;
		file << in.rdbuf();
		/*std::string file((std::istreambuf_iterator<char>(in)), 
						  std::istreambuf_iterator<char>());*/
		input.push(file.str());
		in.close();
		return true;
	}
	return false;
}

void Assembler::setOutputFile(const char* fn) {
	outputFP = fn;
}

void Assembler::tokenize() {
	// Get a line
	std::string ln;
	// This only works for single file processing
	// Need to figure out how to get multiple files working
	std::stringstream s(input.top());
	while(std::getline(s,ln)) {
		// Get tokens from the line
		line toks;
		std::string tok;
		std::stringstream ss(ln);
		while(ss >> tok)
			toks.push_back(tok);

		for(unsigned i=0; i<toks.size(); i++) {
			if(toks[i] == "\t" || toks[i] == "")
				toks.erase(toks.begin()+i);
			else if(toks[i][0] == ';')
				toks.resize(i);
		}
		if(!toks.empty())
			tokens.push_back(toks);
	}
	// DEBUG: print out what has been stored
#ifdef _DEBUG
	for(unsigned i=0; i<tokens.size(); i++) {
		for(unsigned j=0; j<tokens[i].size(); j++) {
			std::cout << tokens[i][j] << "_";
		}
		std::cout << std::endl;
	}
#endif
}

void Assembler::outputFile() {

}

void Assembler::useZeroFill() {
	zeroFill = true;
}

void Assembler::useAlign() {
	alignLabels = true;
}

void Assembler::useCaseSens() {
	caseSens = true;
}

void Assembler::useObsolete() {
	allowObs = true;
}

void Assembler::putMmap() {
	writeMmap = true;
}

bool Assembler::importBin(const std::string& fn, int offs, int n, const std::string& label) {
	return false;
}

void Assembler::writeOp(std::vector<std::string>& tokens) {
	
}

inline void Assembler::op_void(OPCODE op) {
	
}

inline void Assembler::op_imm(OPCODE op,u16 imm) {
	
}

inline void Assembler::op_n(OPCODE op,u8 n) {

}

inline void Assembler::op_n_n(OPCODE op,u8 n1,u8 n2) {

}

inline void Assembler::op_r_imm(OPCODE op,u8 r,u16 imm) {

}

inline void Assembler::op_r_n(OPCODE op,u8 r,u8 n) {

}

inline void Assembler::op_r_r_imm(OPCODE op, u8 r1,u8 r2,u16 imm) {

}

inline void Assembler::op_r_r_r(OPCODE op, u8 r1,u8 r2,u8 r3) {

}

void Assembler::db(std::vector<u8>& bytes) {

}

void Assembler::db(std::string& str) {

}
