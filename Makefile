# VARIABLES

CC = g++
CFLAGS = -O2
D_CFLAGS = -g -D _DEBUG
LDFLAGS = -lm
OBJECTS = main.o Assembler.o Error.o
D_OBJECTS = main.d.o Assembler.d.o Error.d.o

# RELEASE TARGET (DEFAULT)

tchip16: ${OBJECTS}
	${CC} ${CFLAGS} ${OBJECTS} ${LDFLAGS} -s -o $@

main.o: main.cpp
	${CC} -c ${CFLAGS} main.cpp -o main.o

Assembler.o: Assembler.cpp
	${CC} -c ${CFLAGS} Assembler.cpp -o Assembler.o

Error.o: Error.cpp
	${CC} -c ${CFLAGS} Error.cpp -o Error.o

# DEBUG TARGET

debug: tchip16_debug
	
tchip16_debug: ${D_OBJECTS}
	${CC} ${D_CFLAGS} ${D_OBJECTS} ${LDFLAGS} -o $@

# DEBUG OBJECTS

main.d.o: main.cpp
	${CC} -c ${D_CFLAGS} main.cpp -o main.d.o

Assembler.d.o: Assembler.cpp
	${CC} -c ${D_CFLAGS} Assembler.cpp -o Assembler.d.o

Error.d.o: Error.cpp
	${CC} -c ${D_CFLAGS} Error.cpp -o Error.d.o

# ALL TARGETS

all: tchip16 tchip16_debug

# CLEAN TARGET

clean:
	-@rm *.o *.o.d 2> /dev/null || true
