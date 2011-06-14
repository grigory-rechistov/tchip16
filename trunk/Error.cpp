#include <iostream>
#include <cstdlib>

#include "Error.h"


Error::Error(void)
{
	std::cout << "Undefined error\n";
}

Error::Error(int code) {
	print(code);
}

Error::Error(int code, int lineNb) {
	std::cout	<< "At line " << lineNb << ": ";
	print(code);
}

Error::~Error(void)
{
#ifdef _DEBUG
	WAIT
#endif
}

void Error::print(int code) {
	std::cout << "ERROR: ";
	switch(code) {
	case ERR_IO:
		std::cout << "I/O. Please check filenames/permissions #(" << code <<")\n";
		break;
	case ERR_CMD_NONE:
		std::cout	<< "Expected program argument #(" << code << ")\n"
					<< "(possibly missing dest from [-o dest]\n";
		break;
	case ERR_CMD_UNKNOWN:
		std::cout	<< "Unknown program argument #(" << code << ")\n"
					<< "(use -h to see list)\n";
		break;
	case ERR_OP_UNKNOWN:
		std::cout << "Unknown opcode encountered #(" << code << ")\n";
		break;
	case ERR_OP_ARGS:
		std::cout << "Arguments do not match opcode #(" << code << ")\n";
		break;
	case ERR_NUM_NONE:
		std::cout << "Label/constant does not exist #(" << code << ")\n";
		break;
	case ERR_LABEL_REDEF:
		std::cout << "Label already defined #(" << code << ")\n";
		break;
	case ERR_CONST_REDEF:
		std::cout << "Constant already defined #(" << code << ")\n";
		break;
	case ERR_INC_CYCLE:
		std::cout	<< "Import cycle detected #(" << code << ")\n"
					<< "(check your includes for redundancy)\n";
		break;
	default:
		std::cout << "Unknown error encountered #(" << code << ")\n";
		break;
	}
	// Terminate assembler as we have encountered a fatal error
	exit(1);
}
