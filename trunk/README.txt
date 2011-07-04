(C) T Kelsall, 2011 -- All rights reserved.
	See LICENSE.txt for the program license.

------------------------------------------------------------------------------------
CHANGELOG:
------------------------------------------------------------------------------------

See commit notes on project homepage
(http://code.google.com/p/tchip16)
		
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
