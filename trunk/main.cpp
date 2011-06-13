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

/*
	Technical disclaimer:

	tchip16 is my effort to provide a functional and usable assembler
	for the Chip16 system.
	I do NOT claim tchip16 to be optimal(1) or fully-featured(2):
	(1) The assembler is not single-pass or even 2-pass, it is 
		multi-pass. This is done for simplicity, and the small size of
		Chip16 programs combined with the average computer's power
		means this does not affect the user's experience in any noticeable
		fashion.
	(2) This assembler will probably never have the power or flexibility
		of NASM or equivalents; that is not the purpose or the scope of 
		this project, though. This assembler is designed first and for all
		for hobbyist development on the system, not for production 
		environments.
*/
#include "Error.h"
#include "Assembler.h"

int main(int argc, char* argv[]) {
	// Usage: ./tchip16 source [-o dest][-z][-c][-w][-m][-h]
	// source: the input source filename
	// -o: name the output file to dest
	// -z: if assembled code < 64K, zero rest up to 64K
	// -c: make labels and constants case-sensitive
	// -w: display warnings, ie. obsolete opcodes
	// -m: output mmap.txt which displays the address of each label
	// -h: displays help text

	Assembler* tc16 = new Assembler();
	// Parse the command lien arguments
	if(argc > 2) {
		for(int i=2; i<argc; i++) {
			std::string arg(argv[i]);
			if(arg.length() > 1) {
				if(argv[i][1] == 'o') {
					if(argc > i+1)
						tc16->setOutputFile(argv[++i]);
					else
						Error err(ERR_CMD_NONE);
				}
				// ... 
				else if(argv[i][1] == 'h') {

				}
			}
			else
				Error err(ERR_CMD_UNKNOWN);
		}
	}
	return 0;
}