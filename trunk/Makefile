CC = g++
CFLAGS = -Wall -g
LDFLAGS = -lm

Assembler.o: Assembler.cpp Assembler.h
	${CC} ${CFLAGS} -c Assembler.cpp

main.o : main.cpp Assembler.h Error.h
	${CC} ${CFLAGS} -c main.cpp

tchip16: main.o Assembler.o
	${CC} ${CFLAGS} Assembler.o main.o ${LDFLAGS} -o tchip16
