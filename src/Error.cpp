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

bool Error::output = true;

void Error::error(void)
{
    std::cout << "error: undefined\n";
    output = false;
}

void Error::error(ERROR code) {
    std::cout << "error: ";
	print(code);
}

void Error::error(ERROR code, const std::string& fn, int lineNb, const std::string& str) {
	std::cout << fn.c_str() << ":" << lineNb << ": "
		      << "error: " << str.c_str() << ": ";
	print(code);
}

void Error::print(ERROR code) {
	switch(code) {
	case ERR_IO:
		std::cout << "I/O, please check filenames/permissions\n";
		break;
	case ERR_CMD_NONE:
		std::cout	<< "expected program argument "
					<< "(possibly missing dest from [-o dest]?)\n";
		break;
	case ERR_NO_INPUT:
		std::cout	<< "no source file specified "
					<< "(option --help for help)\n";
		break;
	case ERR_CMD_UNKNOWN:
		std::cout	<< "unknown program argument "
					<< "(option --help to see list)\n";
		break;
	case ERR_OP_UNKNOWN:
		std::cout << "unknown opcode encountered\n";
		break;
	case ERR_OP_ARGS:
		std::cout << "arguments do not match opcode\n";
		break;
	case ERR_NUM_NONE:
		std::cout << "label/constant does not exist\n";
		break;
	case ERR_LABEL_REDEF:
		std::cout << "label already defined\n";
		break;
	case ERR_CONST_REDEF:
		std::cout << "constant already defined\n";
		break;
	case ERR_INC_CYCLE:
		std::cout	<< "import cycle detected "
					<< "(file is imported more than once)\n";
		break;
	case ERR_INC_NONE:
		std::cout << "import command missing filename\n";
		break;
	case ERR_TOO_MANY:
		std::cout	<< "too many arguments "
					<< "(see spec for instructions)\n"
					<< "(see readme.txt or run ./tchip16 -h for assembler directives)\n";
		break;
	case ERR_NAN:
		std::cout	<< "not a number "
					<< "(possibly undeclared label)\n";
		break;
	case ERR_NUM_OVERFLOW:
		std::cout	<< "number overflow "
					<< "(value is too large for datatype)\n";
		break;
	case ERR_STR_INVALID:
		std::cout	<< "invalid string encountered "
					<< "(maybe missing a '\"')\n";
		break;
	case ERR_STR_NOLABEL:
		std::cout	<< "string has no label, cannot be referenced\n";
		break;
	default:
		std::cout << "unknown error encountered\n";
		break;
	}
#ifdef _DEBUG
	WAIT;
#endif
    output = false;
}
