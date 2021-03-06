/*    
    tchip16, an open-source Chip16 assembler
    Copyright (C) 2010-2012  Tim Kelsall
    [...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include "Assembler.h"
#include "RomHeader.h"
#include "crc.h"

extern const char* tchip16_ver;

Assembler::Assembler() {
    // Initialize
    initMaps();
    lineNb = 0;
    curAddress = 0;
    totalBytes = 0;
    verbose = false;
    zeroFill = false;
    alignLabels = false;
    writeMmap = false;
    writeHeader = true;
    outputFP = "output.c16";
    start = 0;
    version = 1.1f;
    curB = 0;
    buffer = new u8[MEM_SIZE];
}

Assembler::~Assembler() {
    delete buffer;
}

void Assembler::setOutputFile(const char* fn) {
    outputFP = fn;
}

void Assembler::tokenize(const char* fn) {
    std::string f(fn);
    // Check for import cycles
    for(lineNb=0; lineNb<filesImp.size(); ++lineNb) {
        if(filesImp[lineNb].compare(f) == 0) {
            Error::error(ERR_INC_CYCLE);
            exit(1);
        }
    }
    filesImp.push_back(f);

#ifdef _DEBUG
    std::cout << "Parsing file: " << f << "\n";
#endif

    // Open the file
    std::ifstream file(fn);
    if(!file.is_open()) {
        Error::error(ERR_IO);
        exit(1);
    }
    // Get a line
    std::string ln;
    int lineNbAlt = 0;
    while(std::getline(file,ln)) {
        // Get a string with bad chars in case of db string
        std::string badString;
        int badStart = 0, badEnd = ln.length()-1;
        for( ; badStart<(int)ln.length() && ln[badStart] != '"'; ++badStart){}
        for( ; badEnd>=0 && ln[badEnd] != '"'; --badEnd){}
        badString = ln.substr(badStart, badEnd - badStart + 1);

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
        for(lineNb=0; lineNb<toks.size(); ++lineNb) {
            if(toks[lineNb] == "\t" || toks[lineNb] == "")
                toks.erase(toks.begin()+lineNb);
            else if(toks[lineNb][0] == ';')
                toks.resize(lineNb);
        }
        // Parse some directives
        if(!toks.empty()) {
            if(toks[0] == "include") {
                if(toks.size() == 1)
                    Error::error(ERR_INC_NONE,f,lineNbAlt,toks[0]);
                else if(toks.size() > 2)
                    Error::error(ERR_TOO_MANY,f,lineNbAlt,toks[0]);
                else
                    tokenize(toks[1].c_str());
            }
            else if(toks[0] == "importbin") {
                if(toks.size() < 5)
                    Error::error(ERR_OP_ARGS,f,lineNbAlt,toks[0]);
                else if(toks.size() > 5)
                    Error::error(ERR_TOO_MANY,f,lineNbAlt,toks[0]);
                else if(std::find(labelNames.begin(),labelNames.end(),toks[3]) != labelNames.end())
                    Error::error(ERR_LABEL_REDEF,f,lineNbAlt,toks[3]);
                else {
                    toks.erase(toks.begin(),toks.begin()+1);
                    imports.push_back(toks);
                    labelNames.push_back(toks[3]);
                }
            }
            else if(toks.size() > 1 && toks[1] == "equ") {
                if(toks.size() < 3)
                    Error::error(ERR_OP_ARGS,f,lineNbAlt,toks[1]);
                else if(toks.size() > 3)
                    Error::error(ERR_TOO_MANY,f,lineNbAlt,toks[1]);
                else if(std::find(constNames.begin(),constNames.end(),toks[0]) != constNames.end())
                    Error::error(ERR_CONST_REDEF,f,lineNbAlt,toks[0]);
                else if(toks[2].size() > 2 && toks[2][0] == '$' && toks[2][1] == '-') {
                    unresConsts[toks[0]] =
                        std::make_pair<int,std::string>(lineNbAlt,toks[2].substr(2,toks[2].size()-2));
                }
                else if(atoi_t(toks[2]) > 0xFFFF)
                    Error::error(ERR_NUM_OVERFLOW,f,lineNbAlt,toks[1]);
                else {
                    // Add to map
                    consts[toks[0]] = atoi_t(toks[2]);
                    constNames.push_back(toks[0]);
                }
            }
            else if(toks[0] == "version") {
                if(toks.size() == 1)
                    Error::error(ERR_OP_ARGS,f,lineNbAlt,toks[0]);
                else if(toks.size() > 2)
                    Error::error(ERR_TOO_MANY,f,lineNbAlt,toks[0]);
                else {
                    std::stringstream vss(toks[1]);
                    vss >> version;
                }
            }
            else {
                if(toks[0].size() > 1 &&
                   ((toks[0][0] == ':') || (toks[0][toks[0].size()-1] == ':'))) {
                       std::string label;
                       if(toks[0][0] == ':')
                           label = toks[0].substr(1,toks[0].size()-1);
                       else
                           label = toks[0].substr(0,toks[0].size()-1);
                       int pad = alignLabels ? (totalBytes % 4 != 0 ? 4 - (totalBytes % 4) : 0) : 0;
                       if(consts.find(label) != consts.end())
                           Error::error(ERR_LABEL_REDEF,f,lineNbAlt,label);
                       else {
                           // Add to map
                           consts[label] = totalBytes + pad;
                           // Add to label list
                           labelNames.push_back(label);
                       }
                       // Remove token
                       toks.erase(toks.begin());
                }
                // If after all this there is something left, add it
                if(!toks.empty()) {
                    // Ensure the mnemonic is lowercase
                    std::transform(toks[0].begin(),toks[0].end(),toks[0].begin(),::tolower);
                    // If the mnemonic uses a conditional type, fix it
                    if(toks[0].size() > 1 &&
                        ((toks[0][0] == 'j' && (toks[0] == "jmz" || toks[0][1] != 'm')) ||
                        ((toks[0][0] == 'c') && (toks[0] != "call") && 
                        (toks[0] != "cls") && (toks[0] != "cmpi") && 
                        (toks[0] != "cmp")))) {
                            toks.insert(toks.begin()+1,toks[0].substr(1));
                            toks[0] = toks[0].substr(0,1);
                            toks[0].append("x");
                    }
                    tokens.push_back(toks);
                    lines.push_back(lineNbAlt);
                    files.push_back(std::string(fn));
                    if(toks[0] == "db" && toks.size() > 1) {
                        if(toks[1][0] == '"') {
                            // Drop messed up tokens and inject correct string
                            tokens[tokens.size()-1].resize(1);
                            tokens[tokens.size()-1].push_back(badString);
                            totalBytes += badString.size() - 2;
                            if(!labelNames.empty())
                                stringLines[labelNames.back()] = lineNbAlt;
                            else
                                Error::error(ERR_STR_NOLABEL,fn,lineNbAlt,toks[1]);
                            // Old, hacky way
                            /*for(unsigned i=1; i<toks.size(); ++i)
                                totalBytes += toks[i].size();
                            // Allow for spaces
                            totalBytes += toks.size() - 2;
                            // Remove both quote marks
                            totalBytes -= 2;
                            */
                        }
                        else
                            totalBytes += toks.size() - 1;
                        int pad = alignLabels ? (totalBytes % 4 != 0 ? 4 - (totalBytes % 4) : 0) : 0;
                        totalBytes += pad;
                    }
                    else if(toks[0] == "dw" && toks.size() > 1) {
                        totalBytes += 2*(toks.size() - 1);
                        int pad = alignLabels ? (totalBytes % 4 != 0 ? 4 - (totalBytes % 4) : 0) : 0;
                        totalBytes += pad;
                    }
                    else if(toks[0] != "start")
                        totalBytes += 4;
                }
            }
        }
    }

    file.close();
    // Remember the imports!
    for(unsigned i=0; i<imports.size(); ++i) {
        int pad = alignLabels ? (totalBytes % 4 != 0 ? 4 - (totalBytes % 4) : 0) : 0;
        consts[imports[i][3]] = totalBytes + pad;
        totalBytes += atoi_t(imports[i][2]);
    }
}

void Assembler::outputFile() {
    if(verbose)
        std::cout << "Output binary\n";
    // Output code
    for(lineNb=0; lineNb<tokens.size(); ++lineNb) {
        if(opMap.find(tokens[lineNb][0]) == opMap.end()) {
            Error::error(ERR_OP_UNKNOWN,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            continue;
        }
        u8 opcode = opMap[tokens[lineNb][0]];
        u16 imm;
        u8 n = 0, n1 = 0, n2 = 0;
        switch(opcode) {
        case NOP: case CLS: case VBLNK: case SND0: case PUSHALL: case POPALL: 
        case PUSHF: case POPF: case RET:
            if(tokens[lineNb].size() > 1) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else
                op_void(buffer,opcode);
            break;
        case JMP_I: case JMC: case CALL_I: 
        case SPR: case SND1: case SND2: case SND3: case PAL_I: {
            if(tokens[lineNb].size() > 2 || tokens[lineNb].size() < 2) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on imm
            else if(consts.find(tokens[lineNb][1]) != consts.end()) {
                if(consts[tokens[lineNb][1]] > 0xFFFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][1]);
                    break;
                }
                else
                    imm = consts[tokens[lineNb][1]];
            }
            else
                imm = atoi_t(tokens[lineNb][1]);
            op_imm(buffer,opcode,imm);
            break;
        }
        case Jx: case Cx:
            if(tokens[lineNb].size() > 3 || tokens[lineNb].size() < 3) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on n
            if(condMap.find(tokens[lineNb][1]) != condMap.end())
                n = condMap[tokens[lineNb][1]];
            else {
                Error::error(ERR_OP_UNKNOWN,files[lineNb],lines[lineNb],"j"+tokens[lineNb][1]+" / c"+tokens[lineNb][1]);
                break;
            }
            // Overflow check on imm
            if(consts.find(tokens[lineNb][2]) != consts.end()) {
                if(consts[tokens[lineNb][2]] > 0xFFFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][2]);
                    break;
                }
                imm = consts[tokens[lineNb][2]];
            }
            else
                imm = atoi_t(tokens[lineNb][2]);
            op_n_imm(buffer,opcode,n,imm);
            break;
        case SNG:
            if(tokens[lineNb].size() != 3) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on n
            if(consts.find(tokens[lineNb][1]) != consts.end()) {
                if(consts[tokens[lineNb][1]] > 0xFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][1]);
                    break;
                }
                n = consts[tokens[lineNb][1]];
            }
            else
                n = (u8)atoi_t(tokens[lineNb][1]);
            // Overflow check on imm
            if(consts.find(tokens[lineNb][2]) != consts.end()) {
                if(consts[tokens[lineNb][2]] > 0xFFFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][2]);
                    break;
                }
                imm = consts[tokens[lineNb][2]];
            }
            else
                imm = (u16)atoi_t(tokens[lineNb][2]);
            op_n_imm(buffer,opcode,n,imm);
            break;
        case BGC:
            if(tokens[lineNb].size() > 2 || tokens[lineNb].size() < 2) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on n
            if(consts.find(tokens[lineNb][1]) != consts.end()) {
                if(consts[tokens[lineNb][1]] > 0xFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][1]);
                    break;
                }
                n = consts[tokens[lineNb][1]];
            }
            else
                n = (u8)atoi_t(tokens[lineNb][1]);
            op_n(buffer,opcode,n);
            break;
        case FLIP:
            if(tokens[lineNb].size() > 3 || tokens[lineNb].size() < 3) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on n1
            if(consts.find(tokens[lineNb][1]) != consts.end()) {
                if(consts[tokens[lineNb][1]] > 0xFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][1]);
                    break;
                }
                n1 = consts[tokens[lineNb][1]];
            }
            else
                n1 = (u8)atoi_t(tokens[lineNb][1]);
            // Overflow check on n2
            if(consts.find(tokens[lineNb][2]) != consts.end()) {
                if(consts[tokens[lineNb][2]] > 0xFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][1]);
                    break;
                }
                n2 = consts[tokens[lineNb][1]];
            }
            else
                n2 = (u8)atoi_t(tokens[lineNb][2]);
            op_n_n(buffer,opcode,n1,n2);
            break;
        case CALL_R: case JMP_R: case PUSH: case POP: case PAL_R: case NOT_R: case NEG_R:
            if(tokens[lineNb].size() > 2 || tokens[lineNb].size() < 2) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else if(regMap.find(tokens[lineNb][1]) == regMap.end()) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else 
                op_r(buffer,opcode,regMap[tokens[lineNb][1]]);
            break;
        case SNP: case RND: case LDI_R: case LDI_SP: case LDM_I: case STM_I: case ADDI: case SUBI: 
        case MULI: case DIVI: case NOTI: case NEGI: case MODI: case REMI: case CMPI: case ANDI:
        case TSTI: case ORI: case XORI:
            if(tokens[lineNb].size() > 3 || tokens[lineNb].size() < 3) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on imm
            if(consts.find(tokens[lineNb][2]) != consts.end()) {
                if(consts[tokens[lineNb][2]] > 0xFFFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][2]);
                    break;
                }
                imm = consts[tokens[lineNb][2]];
            }
            else
                imm = atoi_t(tokens[lineNb][2]);
            if(regMap.find(tokens[lineNb][1]) == regMap.end() && tokens[lineNb][1] != "sp" && tokens[lineNb][1] != "SP") {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else if(opcode == LDI_SP)
				op_r_imm(buffer, opcode, 0, imm);
			else
                op_r_imm(buffer,opcode,(u8)regMap[tokens[lineNb][1]],imm);
            break;
        case SHL_N: case SHR_N: case SAR_N:
            if(tokens[lineNb].size() > 3 || tokens[lineNb].size() < 3) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on n
            if(consts.find(tokens[lineNb][2]) != consts.end()) {
                if(consts[tokens[lineNb][2]] > 0xFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][2]);
                    break;
                }
                n = consts[tokens[lineNb][2]];
            }
            else
                n = (u8)atoi_t(tokens[lineNb][2]);
            if(regMap.find(tokens[lineNb][1]) == regMap.end()) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else
                op_r_n(buffer,opcode,(u8)regMap[tokens[lineNb][1]],n);
            break;
        case DRW_I: case JME:
            if(tokens[lineNb].size() > 4 || tokens[lineNb].size() < 4) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Overflow check on imm
            if(consts.find(tokens[lineNb][3]) != consts.end()) {
                if(consts[tokens[lineNb][3]] > 0xFFFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][3]);
                    break;
                }
                imm = consts[tokens[lineNb][3]];
            }
            else
                imm = atoi_t(tokens[lineNb][3]);
            if(regMap.find(tokens[lineNb][1]) == regMap.end() ||
                    regMap.find(tokens[lineNb][2]) == regMap.end()) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else 
                op_r_r_imm(buffer,opcode,(u8)regMap[tokens[lineNb][1]],
                (u8)regMap[tokens[lineNb][2]],imm);
            break;
        case ADD_R2: case SUB_R2: case MUL_R2: case DIV_R2: case AND_R2: case OR_R2:
        case XOR_R2: case SHL_R: case SHR_R: case SAR_R: case LDM_R: case MOV: 
        case NOT_R2: case NEG_R2: case MOD_R2: case REM_R2: case STM_R: case CMP: case TST:
            if(tokens[lineNb].size() > 3 || tokens[lineNb].size() < 3) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else if(regMap.find(tokens[lineNb][1]) == regMap.end() ||
                       regMap.find(tokens[lineNb][2]) == regMap.end()) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else
                op_r_r(buffer,opcode,(u8)regMap[tokens[lineNb][1]],(u8)regMap[tokens[lineNb][2]]);
            break;
        case ADD_R3: case SUB_R3: case MUL_R3: case DIV_R3: case AND_R3: case OR_R3:
        case XOR_R3: case DRW_R: case MOD_R3: case REM_R3:
            if(tokens[lineNb].size() > 4 || tokens[lineNb].size() < 4) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else if(regMap.find(tokens[lineNb][1]) == regMap.end() ||
                    regMap.find(tokens[lineNb][2]) == regMap.end() ||
                    regMap.find(tokens[lineNb][3]) == regMap.end()) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            }
            else 
                op_r_r_r(buffer,opcode,(u8)regMap[tokens[lineNb][1]],
                    (u8)regMap[tokens[lineNb][2]],(u8)regMap[tokens[lineNb][3]]);
            break;
        case DB: {
            if(tokens[lineNb].size() == 1) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            std::vector<u8> vals;
            for(unsigned j=1; j<tokens[lineNb].size(); ++j) {
                u16 val;
                // Overflow check
                if(consts.find(tokens[lineNb][j]) != consts.end()) 
                    val = consts[tokens[lineNb][j]];
                else
                    val = atoi_t(tokens[lineNb][j]);
                if(val > 0xFF) {
                    Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                }
                vals.push_back((u8)val);
            }
            db(buffer,vals);
            break;
                }
        case DW: {
            if(tokens[lineNb].size() == 1) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            std::vector<u16> vals;
            for(unsigned j=1; j<tokens[lineNb].size(); ++j) {
                u16 val;
                // Overflow check
                if(consts.find(tokens[lineNb][j]) != consts.end()) 
                    val = consts[tokens[lineNb][j]];
                else
                    val = atoi_t(tokens[lineNb][j]);
                vals.push_back((u16)val);
            }
            dw(buffer,vals);
            break;
                 }
        case DB_STR: {
            if(tokens[lineNb].size() == 1) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            tokens[lineNb][1] = tokens[lineNb][1].substr(1,tokens[lineNb][1].length()-2);
            if(tokens[lineNb][1] == "")
                Error::error(ERR_STR_INVALID,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            db(buffer,tokens[lineNb][1]);
            break;
                     }
        case START: {
            if(tokens[lineNb].size() == 1) {
                Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            else if(tokens[lineNb].size() > 2) {
                Error::error(ERR_TOO_MANY,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            }
            // Resolve
            if(consts.find(tokens[lineNb][1]) != consts.end()) 
                start = consts[tokens[lineNb][1]];
            else
                start = atoi_t(tokens[lineNb][1]);
            break;
            }

        default:
            Error::error(ERR_OP_UNKNOWN,files[lineNb],lines[lineNb],tokens[lineNb][0]);
            break;
        }
    }
    if(verbose) {
        std::cout << "Output imports\n";
    }
    // Output imported binaries
    for(unsigned i=0; i<imports.size(); ++i) {
        int size = atoi_t(imports[i][2]);
        u8* buf = buffer + curB;
        std::ifstream imp(imports[i][0].c_str(),std::ios::in|std::ios::binary);
        if(!imp.is_open()) {
            Error::error(ERR_IO,std::string(""),0,imports[i][0]);
            break;
        }
        imp.seekg(atoi_t(imports[i][1]));
        imp.read((char*)buf,size);
        imp.close();
        curB += size;
    }
    // If -z, fill with 0's up to 64K
    if(zeroFill) {
        u8* buf = buffer + curB;
        if(verbose)
            std::cout << "Zero memory up to 64K\n";
        for(int i=0; i<0x10000-totalBytes; ++i)
            buf[i] = 0;
    }
    
    if(Error::output) {
        std::ofstream out(outputFP.c_str(),std::ios::out|std::ios::binary);
        if(!out.is_open()) {
            Error::error(ERR_IO,outputFP,0,std::string("All"));
            return;
        }
        
        // Output header
        if(writeHeader) {
            double frac = modf(version,&version);
            u8 ver =  ((u8)(version) << 4) | (u8)(frac*10);
            ch16_header header;
            header.magic = 0x36314843;
            header.reserved = 0x00;
            header.spec_ver = ver;
            header.rom_size = curB;
            crc_t c = crc_init();
            c = crc_update(c,buffer,curB);
            c = crc_finalize(c);
            header.start_addr = start;
            header.crc32_sum = c;
            
            out.write((char*)&header,sizeof(ch16_header));
        }
        
        out.write((char*)buffer,curB);
        out.close();
        
        // If -m, output mmap.txt
        if(writeMmap) {
            if(verbose)
                std::cout << "Output mmap.txt\n";
            std::ofstream mmap("mmap.txt");
            if(mmap.is_open()) {
                mmap    << "Label memory mapping:\n"
                        << "---------------------\n\n";
                std::map<int,std::string> revConsts;
                std::map<std::string,int>::iterator it;
                for(it = consts.begin(); it != consts.end(); ++it)
                    revConsts[it->second] = it->first;
                std::map<int,std::string>::iterator itt;
                for(itt = revConsts.begin(); itt != revConsts.end(); ++itt) {
                    if(std::find(labelNames.begin(),labelNames.end(),itt->second) !=
                            labelNames.end()) {
                        mmap << std::hex << " 0x";
                        char of = mmap.fill('0');
                        mmap.width(4); 
                        mmap << itt->first << " : "  << itt->second << "\n";
                        mmap.fill(of);
                    }
                }
                mmap << "\n---------------------\n";
                mmap.close();
            }
            else
                Error::error(ERR_IO,std::string("All"),0,std::string("mmap.txt"));
        }
    }
}

void Assembler::useVerbose() {
    verbose = true;
    // Say hello then!
    std::cout << tchip16_ver;
}

bool Assembler::isVerbose() {
    return verbose;
}

void Assembler::useZeroFill() {
    zeroFill = true;
}

void Assembler::useAlign() {
    alignLabels = true;
}

void Assembler::putMmap() {
    writeMmap = true;
}

void Assembler::noHeader() {
    writeHeader = false;
}

void Assembler::debugOut() {
    std::cout << "\n-- Debug output information:\n\n";
    if(tokens.empty())
        return;
    std::cout << "Total size: " << totalBytes << "B\n";
    // Print out what files have been used
    std::cout << "\nSource files:\n";
    for(unsigned i=0; i<filesImp.size(); ++i) {
        std::cout << "    " << filesImp[i] << "\n";
    }
    // Print out what has been stored
    std::cout << "\nToken array:\n";
    for(unsigned i=0; i<tokens.size(); ++i) {
        std::cout << "    " << lines[i] << " : ";
        for(unsigned j=0; j<tokens[i].size(); j++) {
            std::cout << "[ " << tokens[i][j] << " ] ";
        }
        std::cout << std::endl;
    }
    // Print out imports
    std::cout << "\nImport list:\n";
    for(unsigned i=0; i<imports.size(); ++i) {
        std::cout << "    ";
        for(unsigned j=0; j<imports[i].size(); j++) {
            std::cout << "[ " << imports[i][j] << " ] ";
        }
        std::cout << std::endl;
    }
    // Print out consts mappings
    std::cout << "\nConsts mapping:\n";
    std::map<std::string,int>::iterator it;
    for(it = consts.begin(); it != consts.end(); it++) {
        std::cout << "    " << std::left << it->first;
        std::cout << std::internal << " : " << it->second;
        if(std::find(labelNames.begin(),labelNames.end(),it->first) != labelNames.end())
            std::cout << std::right << " (label)";
        std::cout << std::endl;
    }
    std::cout << "\n";
}

// Methods that write instructions to disk 

inline void Assembler::op_void(u8* buf, OPCODE op) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = 0; out[2] = 0; out[3] = 0;
    curB += 4;
}

inline void Assembler::op_imm(u8* buf, OPCODE op, u16 imm) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = 0;
    out[2] = imm & 0xFF;
    out[3] = imm >> 8;
    curB += 4;
}

inline void Assembler::op_n_imm(u8* buf, OPCODE op, u8 n, u16 imm) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = n;    
    out[2] = imm & 0xFF;
    out[3] = imm >> 8;
    curB += 4;
}

inline void Assembler::op_n(u8* buf, OPCODE op, u8 n) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = 0;
    out[2] = n; out[3] = 0;
    curB += 4;
}

inline void Assembler::op_n_n(u8* buf, OPCODE op,u8 n1,u8 n2) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = 0; out[2] = 0;
    // Do some error checking
    if(n1 == 0) {
        if(n2 == 0)
            out[3] = 0;
        else if(n2 == 1)
            out[3] = 1;
        else
            Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],std::string("FLIP"));
    }
    else if(n1 == 1) {
        if(n2 == 0)
            out[3] = 2;
        else if(n2 == 1)
            out[3] = 3;
        else
            Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],std::string("FLIP"));
    }
    else
        Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],std::string("FLIP"));

    curB += 4;
}

inline void Assembler::op_r(u8* buf, OPCODE op, u8 r) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = r;
    out[2] = 0; out[3] = 0;
    curB += 4;
}

inline void Assembler::op_r_imm(u8* buf, OPCODE op,u8 r,u16 imm) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = r;
    out[2] = imm & 0xFF;
    out[3] = imm >> 8;
    curB += 4;
}

inline void Assembler::op_r_n(u8* buf, OPCODE op,u8 r,u8 n) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = r;
    out[2] = n; out[3] = 0;
    curB += 4;
}

inline void Assembler::op_r_r(u8* buf, OPCODE op, u8 r1, u8 r2) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = (r2 << 4) | r1;
    out[2] = 0; out[3] = 0;
    curB += 4;
}

inline void Assembler::op_r_r_imm(u8* buf, OPCODE op, u8 r1,u8 r2,u16 imm) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = (r2 << 4) | r1;
    out[2] = imm & 0xFF;
    out[3] = imm >> 8;
    curB += 4;
}

inline void Assembler::op_r_r_r(u8* buf, OPCODE op, u8 r1,u8 r2,u8 r3) {
    u8* out = buf + curB;
    out[0] = op;
    out[1] = (r2 << 4) | r1;
    out[2] = r3;
    out[3] = 0;
    curB += 4;
}

void Assembler::db(u8* buf, std::vector<u8>& bytes) {
    u8* out = buf + curB;
    for(unsigned i=0; i<bytes.size(); ++i) {
        (*out++) = bytes[i];
    }
    curB += bytes.size();
    if(alignLabels && (bytes.size() % 4) != 0) {
        char pad = 0x00;
        for(unsigned i=0; i<4-(bytes.size()%4); ++i) {
            (*out++) = pad;
            ++curB;
        }
    }
}

void Assembler::dw(u8* buf, std::vector<u16>& words) {
    u16* out = (u16*)((u8*)(buf + curB));
    for(unsigned i=0; i<words.size(); ++i) {
        (*out++) = words[i];
    }
    curB += words.size()*2;
    u8* out_ = (u8*)out;
    if(alignLabels && (words.size() % 4) != 0) {
        char pad = 0x00;
        for(unsigned i=0; i<4-(words.size()%4); ++i) {
            (*out_++) = pad;
            ++curB;
        }
    }
}

void Assembler::db(u8* buf, std::string& str) {
    u8* out = buf + curB;
    for(unsigned i=0; i<str.size(); ++i) {
        (*out++) = str[i];
    }
    curB += str.size();
    if(alignLabels && (str.size() % 4) != 0) {
        char pad = 0x00;
        for(unsigned i=0; i<4-(str.size()%4); ++i) {
            (*out++) = pad;
            ++curB;
        }
    }
}

u16 Assembler::atoi_t(std::string str)
{
    if(str.size() == 0)
        return 0;
    std::transform(str.begin(),str.end(),str.begin(),::tolower);
    u16 val = 0, mul = 1;
    // If number is hexadecimal
    // Notation: $NN, #NN, 0xNN, NNh
    if(str.size() > 1 && (
        (str[0] == '#') ||
        (str[0] == '$') ||
        (str[0] == '0' && str[1] == 'x') || 
        (str[str.size()-1] == 'h'))) {
        
        if(str[0] == '#' || str[0] == '$')
            str = str.substr(1,str.size());
        else if(str[0] == '0' && str[1] == 'x')
            str = str.substr(2,str.size());
        else if(str[str.size()-1] == 'h')
            str = str.substr(0,str.size()-1);
        // Number is bigger than 16-bit, not allowed
        if(str.size() > 4)
            Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][0]);
        for(int i=str.size()-1; i>=0; --i) {
            char c = str[i];
            u16 v = 0;
            if(c >= 0x30 && c <= 0x39)
                v = (u16)(c - 0x30);
            else if(c >= 0x61 && c <= 0x66)
                v = (u16)(c - 0x61 + 10);
            else {
                Error::error(ERR_NAN,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                return 0;
            }
            val += mul * v;
            mul *= 16;
        }
        return val;
    }
    // Otherwise, assume it's decimal
    else {
        int start = 0;
        if(str[0] == '-')
            ++start;
        // Number does not fit than 16-bits
        if(str.size() - start > 5)
            Error::error(ERR_NUM_OVERFLOW,files[lineNb],lines[lineNb],tokens[lineNb][0]);
        for(int i=str.size()-1; i>=start; --i) {
            char c = str[i];
            if(c >= 0x30 && c <= 0x39)
                val += mul * (u16)(c - 0x30);
            else {
                Error::error(ERR_NAN,files[lineNb],lines[lineNb],str);
                return 0;
            }
            mul *= 10;
        }
        return start ? (~val+1) : val;
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
    opMap["snp"] = SNP;
    opMap["sng"] = SNG;
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
    opMap["stm_r"] = STM_R;
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
    opMap["modi"] = MODI;
    opMap["mod_r2"] = MOD_R2;
    opMap["mod_r3"] = MOD_R3;
    opMap["remi"] = REMI;
    opMap["rem_r2"] = REM_R2;
    opMap["rem_r3"] = REM_R3;
    opMap["shl_n"] = SHL_N;
    opMap["sal_n"] = SHL_N;
    opMap["shr_n"] = SHR_N;
    opMap["sar_n"] = SAR_N;
    opMap["shl_r"] = SHL_R;
    opMap["sal_r"] = SHL_R;
    opMap["shr_r"] = SHR_R;
    opMap["sar_r"] = SAR_R;
    opMap["push"] = PUSH;
    opMap["pop"] = POP;
    opMap["pushall"] = PUSHALL;
    opMap["popall"] = POPALL;
    opMap["pushf"] = PUSHF;
    opMap["popf"] = POPF;
    opMap["pal_i"] = PAL_I;
    opMap["pal_r"] = PAL_R;
    opMap["noti"] = NOTI;
    opMap["not_r"] = NOT_R;
    opMap["not_r2"] = NOT_R2;
    opMap["negi"] = NEGI;
    opMap["neg_r"] = NEG_R;
    opMap["neg_r2"] = NEG_R2;
    opMap["db_n"] = DB;
    opMap["db_str"] = DB_STR;
    opMap["dw"] = DW;
    opMap["start"] = START;
    // Register mapping
    regMap["r0"] = 0x0; regMap["R0"] = 0x0;
    regMap["r1"] = 0x1; regMap["R1"] = 0x1;
    regMap["r2"] = 0x2; regMap["R2"] = 0x2;
    regMap["r3"] = 0x3; regMap["R3"] = 0x3;
    regMap["r4"] = 0x4; regMap["R4"] = 0x4;
    regMap["r5"] = 0x5; regMap["R5"] = 0x5;
    regMap["r6"] = 0x6; regMap["R6"] = 0x6;
    regMap["r7"] = 0x7; regMap["R7"] = 0x7;
    regMap["r8"] = 0x8; regMap["R8"] = 0x8;
    regMap["r9"] = 0x9; regMap["R9"] = 0x9;
    regMap["ra"] = 0xA; regMap["RA"] = 0xA;
    regMap["rb"] = 0xB; regMap["RB"] = 0xB;
    regMap["rc"] = 0xC; regMap["RC"] = 0xC;
    regMap["rd"] = 0xD; regMap["RD"] = 0xD;
    regMap["re"] = 0xE; regMap["RE"] = 0xE;
    regMap["rf"] = 0xF; regMap["RF"] = 0xF;
    regMap["r10"] = 0xA; regMap["R10"] = 0xA;
    regMap["r11"] = 0xB; regMap["R11"] = 0xB;
    regMap["r12"] = 0xC; regMap["R12"] = 0xC;
    regMap["r13"] = 0xD; regMap["R13"] = 0xD;
    regMap["r14"] = 0xE; regMap["R14"] = 0xE;
    regMap["r15"] = 0xF; regMap["R15"] = 0xF;
    // Condition modes in branching operations
    condMap["z"] = 0x0;
    condMap["mz"] = 0x0;
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
    condMap["mc"] = 0x9;
    condMap["be"] = 0xA;
    condMap["g"] = 0xB;
    condMap["ge"] = 0xC;
    condMap["l"] = 0xD;
    condMap["le"] = 0xE;
    // Mnemonics that need fixing (multiple addressing modes)
    mnemMap["drw"] = drw;
    mnemMap["jmp"] = jmp;
    mnemMap["call"] = call;
    mnemMap["ldi"] = ldi;
    mnemMap["ldm"] = ldm;
    mnemMap["stm"] = stm;
    mnemMap["add"] = add;
    mnemMap["sub"] = sub;
    mnemMap["and"] = _and;
    mnemMap["or"] = _or;
    mnemMap["xor"] = _xor;
    mnemMap["mul"] = mul;
    mnemMap["div"] = _div;
    mnemMap["mod"] = mod; 
    mnemMap["rem"] = rem; 
    mnemMap["shl"] = shl;
    mnemMap["sal"] = sal;
    mnemMap["shr"] = shr;
    mnemMap["sar"] = sar;
    mnemMap["pal"] = pal;
    mnemMap["not"] = _not;
    mnemMap["neg"] = neg; 
    mnemMap["db"] = _db;

}

void Assembler::resolveConsts() {
    for(unresMap::iterator it=unresConsts.begin();
        it!=unresConsts.end(); ++it) {
            // if the string is declared
            if(consts.find(it->second.second) != consts.end()) {
                int line = 0;
                for(unsigned int i=0; i<lines.size(); ++i) {
                    if(lines[i] == stringLines[it->second.second])
                        line = i;
                }
                std::string str(tokens[line][1]);
                // Add the string length to known consts
                consts.insert(std::make_pair(it->first,str.substr(1,str.length()-2).length()));
            }
            else
                Error::error(ERR_NUM_NONE,outputFP,it->second.first,it->second.second);
    }
}

void Assembler::fixOps() {
    for(lineNb=0; lineNb<tokens.size(); ++lineNb) {
        // Ensure all opcodes are lowercase to avoid side-effects
        std::transform(tokens[lineNb][0].begin(),tokens[lineNb][0].begin(),
            tokens[lineNb][0].begin(),::tolower);
        if(opMap.find(tokens[lineNb][0]) == opMap.end()) {
            switch(mnemMap[tokens[lineNb][0]]) {
            case drw:
                if(tokens[lineNb].size() != 4)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][3]) != regMap.end())
                    tokens[lineNb][0] = "drw_r";
                else
                    tokens[lineNb][0] = "drw_i";
                break;
            case jmp:
                if(tokens[lineNb].size() != 2)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][1]) != regMap.end())
                    tokens[lineNb][0] = "jmp_r";
                else
                    tokens[lineNb][0] = "jmp_i";
                break;
            case call:
                if(tokens[lineNb].size() != 2)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][1]) != regMap.end())
                    tokens[lineNb][0] = "call_r";
                else
                    tokens[lineNb][0] = "call_i";
                break;
            case ldi:
                if(tokens[lineNb].size() != 3)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(tokens[lineNb][1][0] == 'r' || tokens[lineNb][1][0] == 'R')
                    tokens[lineNb][0] = "ldi_r";
                else if(tokens[lineNb][1] == "sp" || tokens[lineNb][1] == "SP")
                    tokens[lineNb][0] = "ldi_sp";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][1]);
                break;
            case ldm:
                if(tokens[lineNb].size() != 3)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][2]) != regMap.end())
                    tokens[lineNb][0] = "ldm_r";
                else
                    tokens[lineNb][0] = "ldm_i";
                break;
            case stm:
                if(tokens[lineNb].size() != 3)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][2]) != regMap.end())
                    tokens[lineNb][0] = "stm_r";
                else
                    tokens[lineNb][0] = "stm_i";
                break;
            case add:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "add_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "add_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case sub:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "sub_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "sub_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case _and:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "and_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "and_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case _or:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "or_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "or_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case _xor:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "xor_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "xor_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case mul:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "mul_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "mul_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case _div:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "div_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "div_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case mod:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "mod_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "mod_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case rem:
                if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "rem_r2";
                else if(tokens[lineNb].size() == 4)
                    tokens[lineNb][0] = "rem_r3";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case sal:
            case shl:
                if(tokens[lineNb].size() != 3)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][2]) != regMap.end())
                    tokens[lineNb][0] = "shl_r";
                else
                    tokens[lineNb][0] = "shl_n";
                break;
            case sar:
                if(tokens[lineNb].size() != 3)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][2]) != regMap.end())
                    tokens[lineNb][0] = "sar_r";
                else
                    tokens[lineNb][0] = "sar_n";
                break;
            case shr:
                if(tokens[lineNb].size() != 3)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][2]) != regMap.end())
                    tokens[lineNb][0] = "shr_r";
                else
                    tokens[lineNb][0] = "shr_n";
                break;
            case pal:
                if(tokens[lineNb].size() != 2)
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                if(regMap.find(tokens[lineNb][1]) != regMap.end())
                    tokens[lineNb][0] = "pal_r";
                else
                    tokens[lineNb][0] = "pal_i";
                break;
            case _not:
                if(tokens[lineNb].size() == 2)
                    tokens[lineNb][0] = "not_r";
                else if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "not_r2";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case neg:
                if(tokens[lineNb].size() == 2)
                    tokens[lineNb][0] = "neg_r";
                else if(tokens[lineNb].size() == 3)
                    tokens[lineNb][0] = "neg_r2";
                else
                    Error::error(ERR_OP_ARGS,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                break;
            case _db:
                if(tokens[lineNb][1][0] == '"') {
                    int lastword = tokens[lineNb].size()-1;
                    if(tokens[lineNb][lastword][tokens[lineNb][lastword].size()-1] == '"') {
                        tokens[lineNb][0] = "db_str";
                    }
                    else
                        Error::error(ERR_STR_INVALID,files[lineNb],lines[lineNb],tokens[lineNb][0]);
                }
                else
                    tokens[lineNb][0] = "db_n";
                break;
            default:
                break;
            }
        }
    }
}
