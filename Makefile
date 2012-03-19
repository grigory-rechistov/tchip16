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

obj/main.o: main.cpp Error.h Assembler.h
	${CC} -c ${CFLAGS} main.cpp -o $@ 

obj/Assembler.o: Assembler.cpp Assembler.h Opcodes.h RomHeader.h crc.h
	${CC} -c ${CFLAGS} Assembler.cpp -o $@

obj/Error.o: Error.cpp Error.h
	${CC} -c ${CFLAGS} Error.cpp -o $@ 

obj/crc.o: crc.c crc.h
	${CC} -c ${CFLAGS} crc.c -o $@

# DEBUG TARGET

debug: tchip16_debug
	
tchip16_debug: ${D_OBJECTS}
	${CC} ${D_CFLAGS} ${D_OBJECTS} ${LDFLAGS} -o $@

# DEBUG OBJECTS

obj/main.d.o: main.cpp Error.h Assembler.h
	${CC} -c ${D_CFLAGS} main.cpp -o $@ 

obj/Assembler.d.o: Assembler.cpp Assembler.h Opcodes.h
	${CC} -c ${D_CFLAGS} Assembler.cpp -o $@ 

obj/Error.d.o: Error.cpp Error.h
	${CC} -c ${D_CFLAGS} Error.cpp -o $@ 

obj/crc.d.o: crc.c crc.h
	${CC} -c ${D_CFLAGS} crc.c -o $@ 

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
	-@echo Processing triggers for man-db...
	-@cp tchip16.1.gz /usr/share/man/man1/tchip16.1.gz
	-@mandb > /dev/null || true

uninstall:
	-@echo Removing binaries...
	-@rm /usr/bin/tchip16 /usr/bin/tchip16_debug 2> /dev/null || true
	-@echo Processing triggers for man-db...
	-@rm /usr/share/man/man1/tchip16.1.gz 2> /dev/null || true
	-@mandb > /dev/null || true
