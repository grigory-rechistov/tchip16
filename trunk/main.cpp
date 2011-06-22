/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2011  Tim Kelsall

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>

#include "Error.h"
#include "Assembler.h"

void helpOut();

int main(int argc, char* argv[]) {

	Assembler* tc16 = new Assembler();

	// Parse the command line arguments
	if(argc > 2) {
		for(int i=2; i<argc; i++) {
			std::string arg(argv[i]);
			if(arg.length() > 1 && arg[0] == '-') {
				if(arg[1] == 'o' || arg[1] == 'O') {
					if(argc > i+1)
						tc16->setOutputFile(argv[++i]);
					else
						Error err(ERR_CMD_NONE);
				}
				else if(arg[1] == 'z' || arg[1] == 'Z')
					tc16->useZeroFill();
				else if(arg[1] == 'a' || arg[1] == 'A')
					tc16->useAlign();
				else if(arg[1] == 'c' || arg[1] == 'Z')
					tc16->useCaseSens();
				else if(arg[1] == 'b' || arg[1] == 'B')
					tc16->useObsolete();
				else if(arg[1] == 'm' || arg[1] == 'M')
					tc16->putMmap();
				else if(arg[1] == 'h' || arg[1] == 'H') {
					helpOut();
					return 0;
				}
				else
					Error err(ERR_CMD_UNKNOWN);
			}
			else
				Error err(ERR_CMD_UNKNOWN);
		}
	}

	// Set the input file
	if(argc > 1) {
		if(argv[1][0] == '-' && argv[1][1] == 'h' || argv[1][1] == 'H')
			helpOut();
		else
			tc16->tokenize(argv[1]);
	}
	else
		Error err(ERR_NO_INPUT);

#ifdef _DEBUG
	tc16->debugOut();
	WAIT
#endif

	return 0;
}

void helpOut() {
	std::cout <<	"Usage: ./tchip16 <source> [-o dest][-z][-a][-c][-b][-m][-h]\n\n"
		"\tsource: the input source filename\n"
		"\t-o: name the output file to dest\n"
		"\t-z: if assembled code < 64K, zero rest up to 64K\n"
		"\t-a: align labels to 4-byte boundaries\n"
		"\t-c: make labels and constants case-sensitive\n"
		"\t-b: use backwards compatibility (allow use of obsolete opcodes)\n"
		"\t-m: output mmap.txt which displays the address of each label\n"
		"\t-h: displays help text\n\n";

	std::cout <<	"Directives:\n"
		"\tinclude <file> :\n"
		"\t\tFile is included at this point in the file\n"
		"\t\tFiles may only be included once in the project\n\n"
		"\timportbin <file> <offset> <length> <label> :\n"
		"\t\tFile is read from (offset) to (offset+length), stored at label\n\n"
		"\tconst <name> <value> :\n" 
		"\t\tAll occurrences of name replaced with value\n\n"
		"\tdb <val1> [...] :\n"
		"\t\tStores bytes val1 - ... in the file at this position\n"
		"\t\tAlignment is affected by -a flag\n\n"
		"\tdb <str> :\n" 
		"\t\tStores str (ASCII string) at this position\n\n";

	std::cout << "Key: <> = mandatory, [] = optional\n";
}
