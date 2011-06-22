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

Error::Error(ERROR code, std::string& fn, int lineNb) {
	std::cout << "File: " << fn.c_str() << "\nLine: " << lineNb << "\n";
	print(code);
}

Error::Error(ERROR code, std::string& num) {
	std::cout << "Number " << num.c_str() << ":\n";
	print(code);
}

Error::~Error(void)
{
}

void Error::print(ERROR code) {
	std::cout << "ERROR: ";
	switch(code) {
	case ERR_IO:
		std::cout << "I/O. Please check filenames/permissions #(" << code <<")\n";
		break;
	case ERR_CMD_NONE:
		std::cout	<< "Expected program argument #(" << code << ")\n"
					<< "(possibly missing dest from [-o dest]?)\n";
		break;
	case ERR_NO_INPUT:
		std::cout	<< "No source file specified #(" << code << ")\n"
					<< "(flag -h for help)\n";
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
					<< "(file is imported more than once)\n";
		break;
	case ERR_INC_NONE:
		std::cout << "Import command missing filename #(" << code << ")\n";
		break;
	case ERR_TOO_MANY:
		std::cout	<< "Too many arguments #(" << code << ")\n"
					<< "(see spec for instructions)\n"
					<< "(see readme.txt or run ./tchip16 -h for assembler directives)\n";
		break;
	case ERR_NAN:
		std::cout	<< "Not a number #(" << code << ")\n"
					<< "(possibly undeclared label)\n";
	default:
		std::cout << "Unknown error encountered #(" << code << ")\n";
		break;
	}
#ifdef _DEBUG
	WAIT
#endif
	// Terminate assembler as we have encountered a fatal error
	exit(1);
}
