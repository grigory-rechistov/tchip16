/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Assembler.h"

Assembler::Assembler() {
	input.reserve(20*1024*1024);
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
	
	return false;
}

void Assembler::setOutputFile(const char* fn) {
	outputFP = fn;
}

bool Assembler::importInc(const std::string& fn) {
	
	return false;
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
