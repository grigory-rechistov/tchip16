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

	label1: 	add r0, r1
				sub r0 r2		; Test
				jnz label1
	; Testing
	label2:
				ldi r4,string
				call print_str
	string: 	db "Hello world"
	:vals		db 0x00, 0x01, 0x02, 10, 11h
	print_str:	ret
	
Labels may end or start with a colon ":"
Commas and/or whitespace delimit instructions/operands

------------------------------------------------------------------------------------
DIRECTIVES
------------------------------------------------------------------------------------

tchip16 also features some directives to make your life easier.
