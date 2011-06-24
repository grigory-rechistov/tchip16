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
	// Initialize
	initMaps();
	lineNb = 0;
	curAddress = 0;
	totalBytes = 0;
	zeroFill = false;
	alignLabels = false;
	allowObs = false;
	writeMmap = false;
	outputFP = "output.c16";
	// Say hello
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
	for(unsigned i=0; i<filesImp.size(); ++i) {
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
		for(unsigned i=0; i<toks.size(); ++i) {
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
				imports.push_back(toks);
				totalBytes += atoi_t(toks[3]) - atoi_t(toks[2]);
			}
			else if(toks.size() > 1 && toks[1] == "equ") {
				if(toks.size() < 3)
					Error err(ERR_OP_ARGS,f,lineNbAlt);
				else if(toks.size() > 3)
					Error err(ERR_TOO_MANY,f,lineNbAlt);
				else if(atoi_t(toks[2]) > 0xFFFF)
					Error err(ERR_NUM_OVERFLOW,f,lineNbAlt);
				// Add to map
				consts[toks[0]] = atoi_t(toks[2]);
			}
			else {
				if(toks[0].size() > 1 &&
				   toks[0][0] == ':' || toks[0][toks[0].size()-1] == ':') {
					   // Add to map
					   consts[toks[0].substr(0,toks[0].size()-1)] = totalBytes;
					   // Remove token
					   toks.erase(toks.begin());
				}
				// If after all this there is something left, add it
				if(!toks.empty()) {
					// Ensure the mnemonic is lowercase
					std::transform(toks[0].begin(),toks[0].end(),toks[0].begin(),::tolower);
					// If the mnemonic uses a conditional type, fix it
					if(toks[0].size() > 1 &&
						(toks[0][0] == 'j' && toks[0][1] != 'm') ||
						(toks[0][0] == 'c' && toks[0] != "call" 
							&& toks[0] != "cls" && toks[0] != "cmpi" && toks[0] != "cmp")) {
							toks.insert(toks.begin()+1,toks[0].substr(1));
							toks[0] = toks[0].substr(0,1);
							toks[0].append("x");
					}
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
	std::ofstream out(outputFP,std::ios::out|std::ios::binary);
	if(!out.is_open())
		Error err(ERR_IO,outputFP);
	// Output code
	for(unsigned i=0; i<tokens.size(); ++i) {
		u8 opcode = opMap[tokens[i][0]];
		u16 imm;
		u8 n, n1, n2;
		switch(opcode) {
		case NOP: case CLS: case VBLNK: case SND0: case PUSHALL: case POPALL: case RET:
			if(tokens[i].size() > 1)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			op_void(out,opcode);
			break;
		case JMP_I: case JMC: case CALL_I: 
		case SPR: case SND1: case SND2: case SND3: {
			if(tokens[i].size() > 2 || tokens[i].size() < 2)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			// Overflow check on imm
			if(consts.find(tokens[i][1]) != consts.end()) {
				if(consts[tokens[i][1]] > 0xFFFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][1]);
				imm = consts[tokens[i][1]];
			}
			else
				imm = atoi_t(tokens[i][1]);
			op_imm(out,opcode,imm);
			break;
		}
		case Jx: case Cx:
			if(tokens[i].size() > 3 || tokens[i].size() < 3)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			// Overflow check on n
			if(condMap.find(tokens[i][1]) != condMap.end())
				n = condMap[tokens[i][1]];
			else
				Error err(ERR_OP_UNKNOWN,"j"+tokens[i][1]+" / c"+tokens[i][1]);
			// Overflow check on imm
			if(consts.find(tokens[i][2]) != consts.end()) {
				if(consts[tokens[i][2]] > 0xFFFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][2]);
				imm = consts[tokens[i][1]];
			}
			else
				imm = atoi_t(tokens[i][2]);
			op_n_imm(out,opcode,n,imm);
			break;
		case BGC:
			if(tokens[i].size() > 2 || tokens[i].size() < 2)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			// Overflow check on n
			if(consts.find(tokens[i][1]) != consts.end()) {
				if(consts[tokens[i][1]] > 0xFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][1]);
				n = consts[tokens[i][1]];
			}
			else
				n = (u8)atoi_t(tokens[i][1]);
			op_n(out,opcode,n);
			break;
		case FLIP:
			if(tokens[i].size() > 3 || tokens[i].size() < 3)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			// Overflow check on n1
			if(consts.find(tokens[i][1]) != consts.end()) {
				if(consts[tokens[i][1]] > 0xFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][1]);
				n1 = consts[tokens[i][1]];
			}
			else
				n1 = (u8)atoi_t(tokens[i][1]);
			// Overflow check on n2
			if(consts.find(tokens[i][2]) != consts.end()) {
				if(consts[tokens[i][2]] > 0xFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][1]);
				n2 = consts[tokens[i][1]];
			}
			else
				n2 = (u8)atoi_t(tokens[i][2]);
			op_n_n(out,opcode,n1,n2);
			break;
		case CALL_R: case JMP_R: case PUSH: case POP:
			if(tokens[i].size() > 2 || tokens[i].size() < 2)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			op_r(out,opcode,regMap[tokens[i][1]]);
			break;
		case RND: case LDI_R: case LDI_SP: case LDM_I: case STM_I: case ADDI: case SUBI: 
		case MULI: case DIVI: case CMPI: case ANDI: case TSTI: case ORI: case XORI:
			if(tokens[i].size() > 3 || tokens[i].size() < 3)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			// Overflow check on imm
			if(consts.find(tokens[i][2]) != consts.end()) {
				if(consts[tokens[i][2]] > 0xFFFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][2]);
				imm = consts[tokens[i][2]];
			}
			else
				imm = atoi_t(tokens[i][2]);
			op_r_imm(out,opcode,(u8)regMap[tokens[i][1]],imm);
			break;
		case SHL_N: case SHR_N: case SAR_N:
			if(tokens[i].size() > 3 || tokens[i].size() < 3)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			// Overflow check on n
			if(consts.find(tokens[i][2]) != consts.end()) {
				if(consts[tokens[i][2]] > 0xFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][2]);
				n = consts[tokens[i][2]];
			}
			else
				n = (u8)atoi_t(tokens[i][2]);
			op_r_n(out,opcode,(u8)regMap[tokens[i][1]],n);
			break;
		case DRW_I: case JME:
			if(tokens[i].size() > 4 || tokens[i].size() < 4)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			// Overflow check on imm
			if(consts.find(tokens[i][3]) != consts.end()) {
				if(consts[tokens[i][3]] > 0xFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][3]);
				imm = consts[tokens[i][3]];
			}
			else
				imm = atoi_t(tokens[i][3]);
			op_r_r_imm(out,opcode,(u8)regMap[tokens[i][1]],
				(u8)regMap[tokens[i][2]],imm);
			break;
		case ADD_R2: case SUB_R2: case MUL_R2: case DIV_R2: case AND_R2: case OR_R2:
		case XOR_R2: case SHL_R: case SHR_R: case SAR_R: case LDM_R: case MOV: 
		case STM_R: case CMP: case TST:
			if(tokens[i].size() > 3 || tokens[i].size() < 3)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			op_r_r(out,opcode,(u8)regMap[tokens[i][1]],(u8)regMap[tokens[i][2]]);
			break;
		case ADD_R3: case SUB_R3: case MUL_R3: case DIV_R3: case AND_R3: case OR_R3:
		case XOR_R3: case DRW_R:
			if(tokens[i].size() > 4 || tokens[i].size() < 4)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			op_r_r_r(out,opcode,(u8)regMap[tokens[i][1]],
				(u8)regMap[tokens[i][2]],(u8)regMap[tokens[i][3]]);
			break;
		case DB: {
			if(tokens[i].size() == 1)
				Error err(ERR_OP_ARGS,tokens[i][0]);
			std::vector<u8> vals;
			for(unsigned j=1; j<tokens[i].size(); ++j) {
				u16 val = atoi_t(tokens[i][j]);
				if(val > 0xFF)
					Error err(ERR_NUM_OVERFLOW,tokens[i][0]);
				vals.push_back((u8)val);
			}
			db(out,vals);
			break;
		}
		default:
			Error err(ERR_OP_UNKNOWN,tokens[i][0]);
			break;
		}
	}
	// Output imported binaries
	for(unsigned i=0; i<imports.size(); ++i) {
		int size = atoi_t(imports[i][2]);
		char* buf = new char[size]();
		std::ifstream imp(imports[i][0],std::ios::in|std::ios::binary);
		if(!imp.is_open())
			Error err(ERR_IO,imports[i][0]);
		imp.seekg(atoi_t(imports[i][1]));
		imp.read(buf,size);
		imp.close();
		out.write(buf,size);
	}
	out.close();
}

void Assembler::useZeroFill() {
	zeroFill = true;
}

void Assembler::useAlign() {
	alignLabels = true;
}

void Assembler::useObsolete() {
	allowObs = true;
}

void Assembler::putMmap() {
	writeMmap = true;
}

void Assembler::debugOut() {
	std::cout << "\n-- Debug output information:\n\n";
	if(tokens.empty())
		return;
	std::cout << "Total size: " << totalBytes << "B\n";
	// Print out what files have been used
	std::cout << "Source files:\n";
	for(unsigned i=0; i<filesImp.size(); ++i) {
		std::cout << filesImp[i] << "\n";
	}
	// Print out what has been stored
	std::cout << "\nToken array:\n";
	for(unsigned i=0; i<tokens.size(); ++i) {
		for(unsigned j=0; j<tokens[i].size(); j++) {
			std::cout << "[ " << tokens[i][j] << " ] ";
		}
		std::cout << std::endl;
	}
	// Print out imports
	std::cout << "\nImport list:\n";
	for(unsigned i=0; i<imports.size(); ++i) {
		for(unsigned j=1; j<imports[i].size(); j++) {
			std::cout << "[ " << imports[i][j] << " ] ";
		}
	}
	// Print out consts mappings
	std::cout << "\n\nConsts mapping:\n";
	std::map<std::string,int>::iterator it;
	for(it = consts.begin(); it != consts.end(); it++) {
		std::cout << it->first << " : " << it->second << "\n";
	}
	std::cout << std::endl;
}

// Methods that write instructions to disk 

inline void Assembler::op_void(std::ofstream& bin, OPCODE op) {
	char out[4];
	out[0] = op;
	out[1] = 0; out[2] = 0; out[3] = 0;
	bin.write(out,4);
}

inline void Assembler::op_imm(std::ofstream& bin, OPCODE op, u16 imm) {
	char out[4];
	out[0] = op;
	out[1] = 0;
	out[2] = imm & 0xFF;
	out[3] = imm >> 8;
	bin.write(out,4);
}

inline void Assembler::op_n_imm(std::ofstream& bin, OPCODE op, u8 n, u16 imm) {
	char out[4];
	out[0] = op;
	out[1] = n;	
	out[2] = imm & 0xFF;
	out[3] = imm >> 8;
	bin.write(out,4);
}

inline void Assembler::op_n(std::ofstream& bin, OPCODE op, u8 n) {
	char out[4];
	out[0] = op;
	out[1] = 0;
	out[2] = n; out[3] = 0;
	bin.write(out,4);
}

inline void Assembler::op_n_n(std::ofstream& bin, OPCODE op,u8 n1,u8 n2) {
	char out[4];
	out[0] = op;
	out[1] = 0; out[2] = 0;
	// Do some error checking
	if(n1 == 0) {
		if(n2 == 0)
			out[3] = 0;
		else if(n2 == 1)
			out[3] = 1;
		else
			Error err(ERR_OP_ARGS,std::string("FLIP"));
	}
	else if(n1 == 1) {
		if(n2 == 0)
			out[3] = 2;
		else if(n2 == 1)
			out[3] = 1;
		else
			Error err(ERR_OP_ARGS,std::string("FLIP"));
	}
	else
		Error err(ERR_OP_ARGS,std::string("FLIP"));

	bin.write(out,4);
}

inline void Assembler::op_r(std::ofstream& bin, OPCODE op, u8 r) {
	char out[4];
	out[0] = op;
	out[1] = r;
	out[2] = 0; out[3] = 0;
	bin.write(out,4);
}

inline void Assembler::op_r_imm(std::ofstream& bin, OPCODE op,u8 r,u16 imm) {
	char out[4];
	out[0] = op;
	out[1] = r;
	out[2] = imm & 0xFF;
	out[3] = imm >> 8;
	bin.write(out,4);
}

inline void Assembler::op_r_n(std::ofstream& bin, OPCODE op,u8 r,u8 n) {
	char out[4];
	out[0] = op;
	out[1] = r;
	out[2] = n; out[3] = 0;
	bin.write(out,4);
}

inline void Assembler::op_r_r(std::ofstream& bin, OPCODE op, u8 r1, u8 r2) {
	char out[4];
	out[0] = op;
	out[1] = (r2 << 4) | r1;
	out[2] = 0; out[3] = 0;
	bin.write(out,4);
}

inline void Assembler::op_r_r_imm(std::ofstream& bin, OPCODE op, u8 r1,u8 r2,u16 imm) {
	char out[4];
	out[0] = op;
	out[1] = (r2 << 4) | r1;
	out[2] = imm & 0xFF;
	out[3] = imm >> 8;
	bin.write(out,4);
}

inline void Assembler::op_r_r_r(std::ofstream& bin, OPCODE op, u8 r1,u8 r2,u8 r3) {
	char out[4];
	out[0] = op;
	out[1] = (r2 << 4) | r1;
	out[2] = r3;
	out[3] = 0;
	bin.write(out,4);
}

void Assembler::db(std::ofstream& bin, std::vector<u8>& bytes) {
	for(unsigned i=0; i<bytes.size(); ++i) {
		char out = bytes[i];
		bin.write(&out,1);
	}
}

void Assembler::db(std::ofstream& bin, std::string& str) {

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
			Error err(ERR_NUM_OVERFLOW,str);
		u16 val = 0;
		for(size_t i=0; i<str.size(); ++i) {
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
		for(size_t i=0; i<str.size(); ++i) {
			char c = str[i];
			if(c >= 0x30 && c <= 0x39)
				val += (int)( pow(10.f,(int)(str.size() - i - 1)) ) * (c - 0x30);
			else
				Error err(ERR_NAN,str);
		}
		if(val > 0xFFFF)
			Error err(ERR_NUM_OVERFLOW,str);
		return val;
	}
}

void Assembler::initMaps() {
	// Tedious part: insert all opcodes...
	opMap["nop"] = NOP;
	opMap["cls"] = CLS;
	opMap["vblnk"] = VBLNK;
	opMap["bgc"] = BGC;
	opMap["spr"] = SPR;
	opMap["drw_r"] = DRW_R;
	opMap["drw_i"] = DRW_I;
	opMap["rnd"] = RND;
	opMap["flip"] = FLIP;
	opMap["snd0"] = SND0;
	opMap["snd1"] = SND1;
	opMap["snd2"] = SND2;
	opMap["snd3"] = SND3;
	opMap["jmp_i"] = JMP_I;
	opMap["jmp_r"] = JMP_R;
	opMap["jmc"] = JMC;
	opMap["jx"] = Jx;
	opMap["jme"] = JME;
	opMap["call_i"] = CALL_I;
	opMap["call_r"] = CALL_R;
	opMap["cx"] = Cx;
	opMap["ret"] = RET;
	opMap["ldi_r"] = LDI_R;
	opMap["ldi_sp"] = LDI_SP;
	opMap["ldm_i"] = LDM_I;
	opMap["ldm_r"] = LDM_R;
	opMap["mov"] = MOV;
	opMap["stm_i"] = STM_I;
	opMap["stm"] = STM_R;
	opMap["addi"] = ADDI;
	opMap["add_r2"] = ADD_R2;
	opMap["add_r3"] = ADD_R3;
	opMap["subi"] = SUBI;
	opMap["sub_r2"] = SUB_R2;
	opMap["sub_r3"] = SUB_R3;
	opMap["cmpi"] = CMPI;
	opMap["cmp"] = CMP;
	opMap["andi"] = ANDI;
	opMap["and_r2"] = AND_R2;
	opMap["and_r3"] = AND_R3;
	opMap["tsti"] = TSTI;
	opMap["tst"] = TST;
	opMap["ori"] = ORI;
	opMap["or_r2"] = OR_R2;
	opMap["or_r3"] = OR_R3;
	opMap["xori"] = XORI;
	opMap["xor_r2"] = XOR_R2;
	opMap["xor_r3"] = XOR_R3;
	opMap["muli"] = MULI;
	opMap["mul_r2"] = MUL_R2;
	opMap["mul_r3"] = MUL_R3;
	opMap["divi"] = DIVI;
	opMap["div_r2"] = DIV_R2;
	opMap["div_r3"] = DIV_R3;
	opMap["shl_n"] = SHL_N;
	opMap["sal_n"] = SHL_N;
	opMap["shr_n"] = SHR_N;
	opMap["sar_n"] = SAR_N;
	opMap["shl_r"] = SHL_R;
	opMap["sal_r"] = SHL_R;
	opMap["shr_r"] = SHR_R;
	opMap["sal_r"] = SAR_R;
	opMap["push"] = PUSH;
	opMap["pop"] = POP;
	opMap["pushall"] = PUSHALL;
	opMap["popall"] = POPALL;
	opMap["db"] = DB;
	opMap["db_str"] = DB_STR;

	regMap["r0"] = 0x0;
	regMap["r1"] = 0x1;
	regMap["r2"] = 0x2;
	regMap["r3"] = 0x3;
	regMap["r4"] = 0x4;
	regMap["r5"] = 0x5;
	regMap["r6"] = 0x6;
	regMap["r7"] = 0x7;
	regMap["r8"] = 0x8;
	regMap["r9"] = 0x9;
	regMap["ra"] = 0xA;
	regMap["rb"] = 0xB;
	regMap["rc"] = 0xC;
	regMap["rd"] = 0xD;
	regMap["re"] = 0xE;
	regMap["rf"] = 0xF;
	regMap["r10"] = 0xA;
	regMap["r11"] = 0xB;
	regMap["r12"] = 0xC;
	regMap["r13"] = 0xD;
	regMap["r14"] = 0xE;
	regMap["r15"] = 0xF;

	condMap["z"] = 0x0;
	condMap["nz"] = 0x1;
	condMap["n"] = 0x2;
	condMap["nn"] = 0x3;
	condMap["p"] = 0x4;
	condMap["o"] = 0x5;
	condMap["no"] = 0x6;
	condMap["a"] = 0x7;
	condMap["ae"] = 0x8;
	condMap["nc"] = 0x8;
	condMap["b"] = 0x9;
	condMap["c"] = 0x9;
	condMap["be"] = 0xA;
	condMap["g"] = 0xB;
	condMap["ge"] = 0xC;
	condMap["l"] = 0xD;
	condMap["le"] = 0xE;
	
	mnemMap["drw"] = drw;
	mnemMap["jmp"] = jmp;
	mnemMap["call"] = call;
	mnemMap["ldi"] = ldi;
	mnemMap["ldm"] = ldm;
	mnemMap["stm"] = stm;
	mnemMap["add"] = add;
	mnemMap["sub"] = sub;
	mnemMap["and"] = and;
	mnemMap["or"] = or;
	mnemMap["xor"] = xor;
	mnemMap["mul"] = mul;
	mnemMap["div"] = _div;
	mnemMap["shl"] = shl;
	mnemMap["sal"] = sal;
	mnemMap["shr"] = shr;
	mnemMap["sar"] = sar;

}

void Assembler::fixOps() {
	for(unsigned i=0; i<tokens.size(); ++i) {
		if(opMap.find(tokens[i][0]) == opMap.end()) {
			switch(mnemMap[tokens[i][0]]) {
			case drw:
				if(tokens[i].size() != 4)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][3][0] == 'r')
					tokens[i][0] = "drw_r";
				else
					tokens[i][0] = "drw_i";
				break;
			case jmp:
				if(tokens[i].size() != 2)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][1][0] == 'r')
					tokens[i][0] = "jmp_r";
				else
					tokens[i][0] = "jmp_i";
				break;
			case call:
				if(tokens[i].size() != 2)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][1][0] == 'r')
					tokens[i][0] = "call_r";
				else
					tokens[i][0] = "call_i";
				break;
			case ldi:
				if(tokens[i].size() != 3)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][1][0] == 'r')
					tokens[i][0] = "ldi_r";
				else
					tokens[i][0] = "ldi_sp";
				break;
			case ldm:
				if(tokens[i].size() != 3)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][1][0] == 'r')
					tokens[i][0] = "ldm_r";
				else
					tokens[i][0] = "ldm_i";
				break;
			case stm:
				if(tokens[i].size() != 3)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][1][0] == 'r')
					tokens[i][0] = "stm_r";
				else
					tokens[i][0] = "stm_i";
				break;
			case add:
				if(tokens[i].size() == 3)
					tokens[i][0] = "add_r2";
				else if(tokens[i].size() == 4)
					tokens[i][0] = "add_r3";
				else
					Error err(ERR_OP_ARGS,tokens[i][0]);
				break;
			case sub:
				if(tokens[i].size() == 3)
					tokens[i][0] = "sub_r2";
				else if(tokens[i].size() == 4)
					tokens[i][0] = "sub_r3";
				else
					Error err(ERR_OP_ARGS,tokens[i][0]);
				break;
			case and:
				if(tokens[i].size() == 3)
					tokens[i][0] = "and_r2";
				else if(tokens[i].size() == 4)
					tokens[i][0] = "and_r3";
				else
					Error err(ERR_OP_ARGS,tokens[i][0]);
				break;
			case or:
				if(tokens[i].size() == 3)
					tokens[i][0] = "or_r2";
				else if(tokens[i].size() == 4)
					tokens[i][0] = "or_r3";
				else
					Error err(ERR_OP_ARGS,tokens[i][0]);
				break;
			case xor:
				if(tokens[i].size() == 3)
					tokens[i][0] = "xor_r2";
				else if(tokens[i].size() == 4)
					tokens[i][0] = "xor_r3";
				else
					Error err(ERR_OP_ARGS,tokens[i][0]);
				break;
			case mul:
				if(tokens[i].size() == 3)
					tokens[i][0] = "mul_r2";
				else if(tokens[i].size() == 4)
					tokens[i][0] = "mul_r3";
				else
					Error err(ERR_OP_ARGS,tokens[i][0]);
				break;
			case _div:
				if(tokens[i].size() == 3)
					tokens[i][0] = "div_r2";
				else if(tokens[i].size() == 4)
					tokens[i][0] = "div_r3";
				else
					Error err(ERR_OP_ARGS,tokens[i][0]);
				break;
			case sal:
				tokens[i][0] = "shl";
			case shl:
				if(tokens[i].size() != 3)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][2][0] == 'r')
					tokens[i][0] = "shl_r";
				else
					tokens[i][0] = "shl_n";
				break;
			case sar:
				tokens[i][0] = "shr";
			case shr:
				if(tokens[i].size() != 3)
					Error err(ERR_OP_ARGS,tokens[i][0]);
				if(tokens[i][2][0] == 'r')
					tokens[i][0] = "shr_r";
				else
					tokens[i][0] = "shr_n";
				break;
			default:
				break;
			}
		}
	}
}
