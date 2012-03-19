(C) T Kelsall, 2012 -- All rights reserved.
	See LICENSE.txt for the program license.

http://code.google.com/p/tchip16
------------------------------------------------------------------------------------
CHANGELOG:	V 1.4.3
------------------------------------------------------------------------------------

- see commit comments on project homepage.

------------------------------------------------------------------------------------
INSTALLATION:
------------------------------------------------------------------------------------

LINUX:
    - run `make clean all' to compile.
    - run `sudo make install' to install tchip16 to your /usr/bin, and add a man
      entry for tchip16.
    - run `sudo make uninstall' to remove tchip16 from your /usr/bin and from the 
      man-db.

WINDOWS:
    - consider adding this directory to your PATH, for ease of use.
    - if you have the source distribution, and would like to compile, open the .sln
      file in the tchip16w32/ folder and follow the usual steps (F7 - Build).

------------------------------------------------------------------------------------
USAGE: 
------------------------------------------------------------------------------------

LINUX:    tchip16     <source> [-o dest] [-v|--verbose] [-z|--zero] [-r|--raw]
                               [-a|--align] [-m|--mmap]
          tchip16              [-h|--help] [--version]

WINDOWS:  tchip16.exe <source> [-o dest] [-v|--verbose] [-z|--zero] [-r|--raw]
                               [-a|--align] [-m|--mmap]
          tchip16.exe          [-h|--help] [--version]

Run tchip16 with the --help or -h flag for a description of how they affect your
program.

------------------------------------------------------------------------------------
SYNTAX:
------------------------------------------------------------------------------------

tchip16 accepts standard assembly syntax. Here's an example:

    start       label2
    version     1.0
	constant1	equ 0x1000
	constant2	equ $-string
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
    w_vals:     dw 256, 65535, -2000
	print_str:	ret
	
Labels may end OR start with a colon ":", NOT both
Commas and/or whitespace delimit instructions/operands
0x00, $00, #00, and 00h all denote hex numbers

------------------------------------------------------------------------------------
DIRECTIVES
------------------------------------------------------------------------------------

tchip16 also features some directives to make your life easier:

# START -- start val
Specify the initial value of the PC.

# VERSION -- version M.m
Specify the version of the spec used in the program (default is 1.1).
M = major, m = minor 

# EQU -- name equ val
Allows you to define a constant (name) for use in instructions.
Use $- prefixed to a string name for the length constant of that string (no '\0')

# DB -- db val1 [...]
		db "string"
Allows you to store either bytes or a string at this location in your code.

# DW -- dw val1 [...]
Allows you to store 16-bit words in little endian format at this location in your
code.

# INCLUDE -- include otherfile.s
Allows you to import another source file at this location in your code.
Files may only be included ONCE in the whole project.
I recommend you organize the files so you have one main file which includes
helper files, which in turn should be as self-contained as possible.

# IMPORTBIN -- importbin filename offset n label
Allows you to import a binary file, which will be appended verbatim at the end of
the code. They are stored in the order they are imported.
Imported: filename, from address offset to (offset+n), written from address label
in the ROM.
