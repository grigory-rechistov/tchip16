# VARIABLES

CC = g++
CFLAGS = -Wall -Os 
D_CFLAGS = -Wall -D _DEBUG
LDFLAGS = -lm
OBJECTS = obj/main.o obj/Assembler.o obj/Error.o obj/crc.o
D_OBJECTS = obj/main.d.o obj/Assembler.d.o obj/Error.d.o obj/crc.d.o

.PHONY: all debug clean win windows win_debug windows_debug

#####################################################################
# RELEASE TARGET (DEFAULT)

tchip16: ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} ${LDFLAGS} -s -o $@

obj/main.o: main.cpp
	${CC} -c ${CFLAGS} main.cpp -o obj/main.o

obj/Assembler.o: Assembler.cpp
	${CC} -c ${CFLAGS} Assembler.cpp -o obj/Assembler.o

obj/Error.o: Error.cpp
	${CC} -c ${CFLAGS} Error.cpp -o obj/Error.o

obj/crc.o: crc.c
	${CC} -c ${CFLAGS} crc.c -o obj/crc.o

# DEBUG TARGET

debug: tchip16_debug
	
tchip16_debug: ${D_OBJECTS}
	${CC} ${D_CFLAGS} ${D_OBJECTS} ${LDFLAGS} -o $@

# DEBUG OBJECTS

obj/main.d.o: main.cpp
	${CC} -c ${D_CFLAGS} main.cpp -o obj/main.d.o

obj/Assembler.d.o: Assembler.cpp
	${CC} -c ${D_CFLAGS} Assembler.cpp -o obj/Assembler.d.o

obj/Error.d.o: Error.cpp
	${CC} -c ${D_CFLAGS} Error.cpp -o obj/Error.d.o

obj/crc.d.o: crc.c
	${CC} -c ${D_CFLAGS} crc.c -o obj/crc.d.o

#####################################################################
# ALL TARGETS

all: tchip16 tchip16_debug 

# CLEAN TARGET

clean:
	-@rm obj/*.o 2> /dev/null || true
