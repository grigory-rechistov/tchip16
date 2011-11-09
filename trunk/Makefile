CC = g++
CFLAGS = -O -g
LDFLAGS = -lm
OBJECTS = main.o Assembler.o Error.o

tchip16: main.o Assembler.o Error.o
	${CC} ${CFLAGS} ${OBJECTS} ${LDFLAGS} -s -o tchip16

%.o: %.cpp
	${CC} ${CFLAGS} -c $<

clean:
	-@rm -rf *.o tchip16 2> /dev/null || true
