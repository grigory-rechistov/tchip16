(C) T Kelsall, 2011 -- All rights reserved.
	See LICENSE.txt for the program license.

------------------------------------------------------------------------------------
CHANGELOG:
------------------------------------------------------------------------------------

tchip16 has been rewritten from scratch for this version.
Hopefully there will not be too many bugs -- but there are many improvements, both
to the user and under the hood, including:

- new "include" directive, source files may be included to modularize programs
- new "db" directive, can be used to store values or strings
- new "equ" directive, can be used for named constants
- more flexible syntax -- for instance, labels may be placed on the same line as an
  instruction
- new error mechanism, with error codes and descriptions of the problem (including
  line number, file and/or value when appropriate)
- command line arguments, modifying the way the file is compiled and/or displaying
  help text

------------------------------------------------------------------------------------
TECHNICAL DISCLAIMER:
------------------------------------------------------------------------------------

	tchip16 is my effort to provide a functional and usable assembler
	for the Chip16 system.
	I do NOT claim tchip16 to be optimal(1) or fully-featured(2):
	(1) The assembler was programmed for simplicity, and the small size of
		Chip16 programs combined with the average computer's power
		means this does not affect the user's experience in any noticeable
		fashion.
	(2) This assembler will probably never have the power or flexibility
		of NASM or equivalents; that is not the purpose or the scope of 
		this project, though. This assembler is designed first and for all
		for hobbyist development on the system, not for production 
		environments.
		
------------------------------------------------------------------------------------
USAGE: 
------------------------------------------------------------------------------------

LINUX:		./tchip16 <source> [-o dest][-z][-a][-c][-b][-m][-h]
WINDOWS:	tchip16.exe <source> [-o dest][-z][-a][-c][-b][-m][-h]

Run tchip16 with the -h ("help") flag for a description of how they affect your program.

------------------------------------------------------------------------------------
SYNTAX:
------------------------------------------------------------------------------------

tchip16 accepts standard assembly syntax. Here's an example:

	constant1	equ 0x1000
	lucky		equ 7
	
	label1: 	add r0, r1
				muli r0, lucky
				sub r0 r2		; Test
				jnz label1
	; Testing
	label2:
				ldi r4,string
				call print_str
	string: 	db "Hello world"
	:vals		db 0x00, $01, #02, 10, 11h
	print_str:	ret
	
Labels may end OR start with a colon ":", NOT both
Commas and/or whitespace delimit instructions/operands
0x00, $00, #00, and 00h all denote hex numbers

------------------------------------------------------------------------------------
DIRECTIVES
------------------------------------------------------------------------------------

tchip16 also features some directives to make your life easier:

# EQU -- name equ val
Allows you to define a constant (name) for use in instructions.

# DB -- db val1 [...]
		db "string"
Allows you to store either bytes or a string at this location in your code.

# INCLUDE -- include otherfile.s
Allows you to import another source file at this location in your code.
Files may only be included ONCE in the whole project.
I recommend you organize the files so you have one main file which includes
helper files, which in turn should be as self-contained as possible.

# IMPORTBIN -- importbin filename offset n label
Allows you to import a binary file, which will be appended verbatim at the end of
the code. They are stored in the order they are imported.
Imported: filename, from address offset to (offset+n), starting at label
