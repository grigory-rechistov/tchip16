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

const char* tchip16_ver = "tchip16 1.4.3 -- a chip16 assembler\n";

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

    bool validCmds = true;

	// Parse the command line arguments
	if(argc > 2) {
		for(int i=1+nbFiles; i<argc; ++i) {
			std::string arg(argv[i]);
			if(arg.length() > 1 && arg[0] == '-') {
				if(arg[1] == 'o' || arg[1] == 'O') {
					if(argc > i+1)
						tc16->setOutputFile(argv[++i]);
					else
						Error::error(ERR_CMD_NONE);
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
                else if(arg == "--version") {
                    std::cout << tchip16_ver;
                    return 0;
                }
                else if(arg == "--dog") {
                    std::cout << "HELLO\nYES, THIS IS DOG\n";
                    return 0;
                }
				else {
					Error::error(ERR_CMD_UNKNOWN);
                    validCmds = false;
                }
			}
			else {
				Error::error(ERR_CMD_UNKNOWN);
                validCmds = false;
            }
		}
	}
    
    if(!validCmds)
        return 1;

	// Set the input file
	if(argc > 1) {
        std::string arg(argv[1]);
		if(arg == "-h" || arg == "H" || arg == "--help") {
            helpOut();
			return 0;
		}
        else if(arg == "--version") {
            std::cout << tchip16_ver;
            return 0;
        }
        else if(arg == "--dog") {
            std::cout << "HELLO\nYES, THIS IS DOG\n";
            return 0;
        }
	}
	else {
		Error::error(ERR_NO_INPUT);
        return 1;
    }
#ifdef _DEBUG
	tc16->useVerbose();
#endif
    // Do stuff!
    for(int i=0; i<nbFiles; ++i)
        tc16->tokenize(argv[1+i]);
    if(tc16->isVerbose())
        std::cout << "Built tokens\n";
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
	std::cout << 
        "Usage: tchip16 SOURCE... [OPTION]...\n\n"
		"    Assemble SOURCE(s) to produce a chip16 binary.\n\n"
        "File options:\n\n"
		"    -o DEST: output file is DEST\n"
		"    -a, --align: align labels to 4-byte boundaries\n"
		"    -z, --zero: if assembled code < 64K, zero rest up to 64K\n"
        "    -r, --raw: do not output header, only raw chip16 ROM\n\n"
		"Information options:\n\n"
        "    -m, --mmap: output mmap.txt which displays the address of each label\n"
		"    -v, --verbose: switch to verbose output (default is silent)\n\n"
        "Miscellaneous options:\n\n"
		"    -h, --help: display this help text and exit\n"
        "    --version: display version information and exit\n\n"
        "Assembler directives: see README.txt\n"
        "Copyright (C) 2010-12 tykel\n"
        "http://code.google.com/p/tchip16\n";
}
