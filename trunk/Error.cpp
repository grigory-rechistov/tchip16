/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2010-2012  Tim Kelsall
	[...]
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <cstdlib>

#include "Error.h"


Error::Error(void)
{
	std::cout << "Undefined error\n";
}

Error::Error(ERROR code) {
	print(code);
}

Error::Error(ERROR code, const std::string& fn, int lineNb, const std::string& str) {
	std::cout << fn.c_str() << ":" << lineNb << ": "
		      << "error: (" << str.c_str() << ") ";
	print(code);
}

Error::~Error(void)
{
}

void Error::print(ERROR code) {
	switch(code) {
	case ERR_IO:
		std::cout << "I/O. Please check filenames/permissions\n";
		break;
	case ERR_CMD_NONE:
		std::cout	<< "Expected program argument "
					<< "(possibly missing dest from [-o dest]?)\n";
		break;
	case ERR_NO_INPUT:
		std::cout	<< "No source file specified "
					<< "(flag -h for help)\n";
		break;
	case ERR_CMD_UNKNOWN:
		std::cout	<< "Unknown program argument "
					<< "(use -h to see list)\n";
		break;
	case ERR_OP_UNKNOWN:
		std::cout << "Unknown opcode encountered\n";
		break;
	case ERR_OP_ARGS:
		std::cout << "Arguments do not match opcode\n";
		break;
	case ERR_NUM_NONE:
		std::cout << "Label/constant does not exist\n";
		break;
	case ERR_LABEL_REDEF:
		std::cout << "Label already defined\n";
		break;
	case ERR_CONST_REDEF:
		std::cout << "Constant already defined\n";
		break;
	case ERR_INC_CYCLE:
		std::cout	<< "Import cycle detected "
					<< "(file is imported more than once)\n";
		break;
	case ERR_INC_NONE:
		std::cout << "Import command missing filename\n";
		break;
	case ERR_TOO_MANY:
		std::cout	<< "Too many arguments "
					<< "(see spec for instructions)\n"
					<< "(see readme.txt or run ./tchip16 -h for assembler directives)\n";
		break;
	case ERR_NAN:
		std::cout	<< "Not a number "
					<< "(possibly undeclared label)\n";
		break;
	case ERR_NUM_OVERFLOW:
		std::cout	<< "Number overflow "
					<< "(value is too large for datatype)\n";
		break;
	case ERR_STR_NOTENDED:
		std::cout	<< "String not ended "
					<< "(missing a \")\n";
		break;
	case ERR_STR_EMPTY:
		std::cout	<< "Empty string is illegal\n";
		break;
	case ERR_STR_NOLABEL:
		std::cout	<< "String has no label, cannot be referenced\n";
		break;
	default:
		std::cout << "Unknown error encountered\n";
		break;
	}
#ifdef _DEBUG
	WAIT;
#endif
	// Terminate assembler as we have encountered a fatal error
	exit(1);
}
