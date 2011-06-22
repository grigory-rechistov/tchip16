/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Assembler.h"

Assembler::Assembler() {
	buffer = new u8[MEM_SIZE]();
	lineNb = 0;
	curAddress = 0;
	totalBytes = 0;
	zeroFill = false;
	alignLabels = false;
	caseSens = false;
	allowObs = false;
	writeMmap = false;

	std::cout	<< "tchip16 -- a Chip16 assembler\n"
				<< "(C) 2011 tykel\n\n";
}

Assembler::~Assembler() {

}

void Assembler::setOutputFile(const char* fn) {
	outputFP = fn;
}

void Assembler::tokenize(const char* fn) {
	std::string f(fn);
	// Check for import cycles
	for(unsigned i=0; i<filesImp.size(); i++) {
		if(filesImp[i].compare(f) == 0)
			Error err(ERR_INC_CYCLE);
	}
	filesImp.push_back(f);

#ifdef _DEBUG
	std::cout << "Parsing file: " << f << "\n";
#endif

	// Open the file
	std::ifstream file(fn);
	if(!file.is_open())
		Error err(ERR_IO);
	// Get a line
	std::string ln;
	int lineNbAlt = -1;
	while(std::getline(file,ln)) {
		lineNbAlt++;
		// Strip ',' from the string
		std::replace(ln.begin(),ln.end(),',',' ');
		// Get tokens from the line
		line toks;
		std::string tok;
		std::stringstream ss(ln);
		while(ss >> tok)
			toks.push_back(tok);
		// Some basic parsing
		for(unsigned i=0; i<toks.size(); i++) {
			if(toks[i] == "\t" || toks[i] == "")
				toks.erase(toks.begin()+i);
			else if(toks[i][0] == ';')
				toks.resize(i);
		}
		// Parse some directives
		if(!toks.empty()) {
			if(toks[0] == "include") {
				if(toks.size() == 1)
					Error err(ERR_INC_NONE,f,lineNbAlt);
				else if(toks.size() > 2)
					Error err(ERR_TOO_MANY,f,lineNbAlt);
				else
					tokenize(toks[1].c_str());
			}
			else if(toks[0] == "importbin") {
				if(toks.size() < 5)
					Error err(ERR_OP_ARGS,f,lineNbAlt);
				else if(toks.size() > 5)
					Error err(ERR_TOO_MANY,f,lineNbAlt);
				//TODO: importing file!
				totalBytes += atoi_t(toks[3]) - atoi_t(toks[2]);
			}
			else if(toks.size() > 1 && toks[1] == "equ") {
				if(toks.size() < 3)
					Error err(ERR_OP_ARGS,f,lineNbAlt);
				else if(toks.size() > 3)
					Error err(ERR_TOO_MANY,f,lineNbAlt);
				consts[toks[0]] = atoi_t(toks[2]);
			}
			else {
				if(toks[0].size() > 1 &&
				   toks[0][0] == ':' || toks[0][toks[0].size()-1] == ':') {
					   labels[toks[0]] = totalBytes;
				}
				if(!toks.empty()) {
					tokens.push_back(toks);
					lineNb++;
					totalBytes += 4;
				}
			}
		}
	}

	file.close();
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

void Assembler::debugOut() {
	if(tokens.empty())
		return;
	std::cout << "Total size: " << totalBytes << "B\n";
	// Print out what has been stored
	std::cout << "\n\nToken array:\n";
	for(unsigned i=0; i<tokens.size(); i++) {
		for(unsigned j=0; j<tokens[i].size(); j++) {
			std::cout << "[ " << tokens[i][j] << " ] ";
		}
		std::cout << std::endl;
	}
	// Print out label mappings
	std::cout << "\n\nLabel mapping:\n";
	std::map<std::string,int>::iterator it;
	for(it = labels.begin(); it != labels.end(); it++) {
		std::cout << it->first << " : " << it->second << "\n";
	}
	// Print out equ mappings
	std::cout << "\n\nEqu mapping:\n";
	for(it = consts.begin(); it != consts.end(); it++) {
		std::cout << it->first << " : " << it->second << "\n";
	}
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

u16 Assembler::atoi_t(std::string& str)
{
	if(str.size() == 0)
		return 0;
	std::transform(str.begin(),str.end(),str.begin(),::toupper);
	// If number is hexadecimal
	// Notation: $NN, #NN, 0xNN, NNh
	if(str.size() > 1 && (
		(str[0] == '#') ||
		(str[0] == '$') ||
		(str[0] == '0' && str[1] == 'X') || 
		(str[str.size()-1] == 'H'))) {

		if(str[0] == '#' || str[0] == '$')
			str = str.substr(1,str.size());
		else if(str[0] == '0' && str[1] == 'X')
			str = str.substr(2,str.size());
		else if(str[str.size()-1] == 'H')
			str = str.substr(0,str.size()-1);
		// Number is bigger than 16-bit, not allowed
		if(str.size() > 4)
			str = str.substr(0,4);
		u16 val = 0;
		for(size_t i=0; i<str.size(); i++) {
			char c = str[i];
			if(c >= 0x30 && c <= 0x39)
				val += (int)( pow(16.f,(int)(str.size() - i - 1)) ) * (c - 0x30);
			else if(c >= 0x41 && c <= 0x46)
				val += (int)( pow(16.f,(int)(str.size() - i - 1)) ) * (c - 0x41 + 10);
			else 
				Error err(ERR_NAN,str);
		}
		return val;
	}
	// Otherwise, assume it's decimal
	else {
		u16 val = 0;
		for(size_t i=0; i<str.size(); i++) {
			char c = str[i];
			if(c >= 0x30 && c <= 0x39)
				val += (int)( pow(10.f,(int)(str.size() - i - 1)) ) * (c - 0x30);
			else
				Error err(ERR_NAN,str);
		}
		return val;
	}
}
