/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Assembler.h"

Assembler::Assembler() {
	buffer = new u8[MEM_SIZE]();
	lineNb = 0;
	curAddress = 0;
}

Assembler::~Assembler() {

}

bool Assembler::openFile(const char* fn) {
	std::ifstream file(fn);
	if(file.is_open()) {
		input = new std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		if(input != NULL)
			return true;
	}
	return false;
}

void Assembler::setOutputFile(const char* fn) {
	outputFP = fn;
}

bool Assembler::importInc(const std::string& fn) {
	std::ifstream inc(fn.c_str());
	if(inc.is_open()) {
		std::string incFile((std::istreambuf_iterator<char>(inc)), std::istreambuf_iterator<char>());
		input->append(incFile);
	}
	return false;
}

void Assembler::writeOp(std::vector<std::string>& tokens) {
	
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
