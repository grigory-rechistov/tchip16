/*    
	tchip16, an open-source Chip16 assembler
    Copyright (C) 2010-12  Tim Kelsall

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

	int nbFiles = 0;

	// Source of a silly bug -- was only checking if argc > 2 (doesn't work with lone arg)
	if(argc > 1) {
		for(int i=1; i<argc; ++i) {
			if(argv[i][0] != '-' && argv[i-1][0] != '-')
				++nbFiles;
			else
				break;
		}
	}

	// Parse the command line arguments
	if(argc > 2) {
		for(int i=1+nbFiles; i<argc; ++i) {
			std::string arg(argv[i]);
			if(arg.length() > 1 && arg[0] == '-') {
				if(arg[1] == 'o' || arg[1] == 'O') {
					if(argc > i+1)
						tc16->setOutputFile(argv[++i]);
					else
						Error err(ERR_CMD_NONE);
				}
				else if(arg == "-v" || arg == "-V" || arg == "--verbose")
					tc16->useVerbose();
				else if(arg == "-z" || arg == "-Z" || arg == "--zero")
					tc16->useZeroFill();
				else if(arg == "-a" || arg == "-A" || arg == "--align")
					tc16->useAlign();
				else if(arg == "-m" || arg == "-M" || arg == "--mmap")
					tc16->putMmap();
                else if(arg == "-r" || arg == "-R" || arg == "--raw")
                    tc16->noHeader();
				else if(arg == "-h" || arg == "-H" || arg == "--help") {
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
		if(argv[1][0] == '-' && argv[1][1] == 'h' || argv[1][1] == 'H') {
			// No need to output or how debug info, so return from here
            tc16->useVerbose();     // Output the version
			helpOut();
			return 0;
		}
	}
	else
		Error err(ERR_NO_INPUT);
#ifdef _DEBUG
	tc16->useVerbose();
#endif
    // Do stuff!
    for(int i=0; i<nbFiles; ++i)
        tc16->tokenize(argv[1+i]);
    if(tc16->isVerbose())
        std::cout << "Building tokens... OK \n";
	tc16->fixOps();
	tc16->resolveConsts();
#ifdef _DEBUG
	tc16->debugOut();
#endif
	tc16->outputFile();
	if(tc16->isVerbose())
		std::cout << "\nBuild complete.\n";

#ifdef _DEBUG
	WAIT;
#endif
	return 0;
}

void helpOut() {
	std::cout << "Usage: tchip16 <source> [-o dest][-v][-z][-a][-m][-h]\n\n"
		"    source: the input source filename\n"
		"    -v: switch to verbose output (default is silent)\n"
		"    -o: name the output file to dest\n"
		"    -z: if assembled code < 64K, zero rest up to 64K\n"
		"    -a: align labels to 4-byte boundaries\n"
		"    -m: output mmap.txt which displays the address of each label\n"
		"    -h: displays help text\n\n";

	std::cout << "Directives:\n"
		"    include <file> :\n"
		"        File is included for use in current file\n"
		"        Files may only be included once in the project\n\n"
		"    importbin <file> <offset> <length> <label> :\n"
		"        File is read from (offset) to (offset+length), stored at label\n\n"
		"    <name> equ <value> :\n" 
		"        All occurrences of name replaced with value\n\n"
		"    db <val1> [...] :\n"
		"        Stores bytes val1 - ... in the file at this position\n"
		"        Alignment is affected by -a flag\n\n"
        "    dw <val1> [...] : \n"
        "        Stores words val1 - ... in the file at this position (little-endian)\n"
        "        Alignment is affected by -a flag\n\n"
		"    db <str> :\n" 
		"        Stores str (ASCII string) at this position\n\n";

	std::cout << "Key: <> = mandatory, [] = optional\n";
}
