# VARIABLES

CC = g++
CFLAGS = -Wall -Os
D_CFLAGS = -Wall -D _DEBUG
LDFLAGS = -lm
OBJECTS = obj/main.o obj/Assembler.o obj/Error.o obj/crc.o
D_OBJECTS = obj/main.d.o obj/Assembler.d.o obj/Error.d.o obj/crc.d.o

.PHONY: all debug clean

#####################################################################
# RELEASE TARGET (DEFAULT)

tchip16: ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} ${LDFLAGS} -s -o $@

obj/main.o: src/main.cpp src/Error.h src/Assembler.h
	${CC} -c ${CFLAGS} src/main.cpp -o $@ 

obj/Assembler.o: src/Assembler.cpp src/Assembler.h src/Opcodes.h src/RomHeader.h src/crc.h
	${CC} -c ${CFLAGS} src/Assembler.cpp -o $@

obj/Error.o: src/Error.cpp src/Error.h
	${CC} -c ${CFLAGS} src/Error.cpp -o $@ 

obj/crc.o: src/crc.c src/crc.h
	${CC} -c ${CFLAGS} src/crc.c -o $@

# DEBUG TARGET

debug: tchip16_debug
	
tchip16_debug: ${D_OBJECTS}
	${CC} ${D_CFLAGS} ${D_OBJECTS} ${LDFLAGS} -o $@

# DEBUG OBJECTS

obj/main.d.o: src/main.cpp src/Error.h src/Assembler.h
	${CC} -c ${D_CFLAGS} src/main.cpp -o $@ 

obj/Assembler.d.o: src/Assembler.cpp src/Assembler.h src/Opcodes.h
	${CC} -c ${D_CFLAGS} src/Assembler.cpp -o $@ 

obj/Error.d.o: src/Error.cpp src/Error.h
	${CC} -c ${D_CFLAGS} src/Error.cpp -o $@ 

obj/crc.d.o: src/crc.c src/crc.h
	${CC} -c ${D_CFLAGS} src/crc.c -o $@ 

#####################################################################
# ALL TARGETS

all: tchip16 tchip16_debug 

# CLEAN TARGET

clean:
	-@rm obj/*.o tchip16 tchip16_debug 2> /dev/null || true

# (UN)INSTALL TARGET

install:
	-@echo Installing binaries...
	-@cp tchip16 tchip16_debug /usr/bin
	-@echo Installing man page... 
	-@cp src/tchip16.1.gz /usr/share/man/man1/tchip16.1.gz
	-@echo Updating man-db...
	-@mandb -q #> /dev/null || true
	-@echo done.

uninstall:
	-@echo Removing binaries...
	-@rm /usr/bin/tchip16 /usr/bin/tchip16_debug 2> /dev/null || true
	-@echo Removing man page...
	-@rm /usr/share/man/man1/tchip16.1.gz 2> /dev/null || true
	-@echo Updating man-db... 
	-@mandb -q #> /dev/null || true
	-@echo done.
