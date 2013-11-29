# VARIABLES

USER=$(shell whoami)
CC = g++
CFLAGS = -Wall -O0 -g
D_CFLAGS = -Wall -D _DEBUG
LDFLAGS = -lm
SRCDIR = src
OBJDIR = obj
OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/Assembler.o $(OBJDIR)/Error.o $(OBJDIR)/crc.o
D_OBJECTS = $(OBJDIR)/main.d.o $(OBJDIR)/Assembler.d.o $(OBJDIR)/Error.d.o $(OBJDIR)/crc.d.o

.PHONY: all debug clean install uninstall

#####################################################################
# RELEASE TARGET (DEFAULT)

tchip16: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp $(SRCDIR)/Error.h $(SRCDIR)/Assembler.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/main.cpp -o $@ 

$(OBJDIR)/Assembler.o: $(SRCDIR)/Assembler.cpp $(SRCDIR)/Assembler.h $(SRCDIR)/Opcodes.h $(SRCDIR)/RomHeader.h $(SRCDIR)/crc.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/Assembler.cpp -o $@

$(OBJDIR)/Error.o: $(SRCDIR)/Error.cpp $(SRCDIR)/Error.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/Error.cpp -o $@ 

$(OBJDIR)/crc.o: $(SRCDIR)/crc.c $(SRCDIR)/crc.h
	$(CC) -c $(CFLAGS) $(SRCDIR)/crc.c -o $@

# DEBUG TARGET

debug: tchip16_debug
	
tchip16_debug: $(D_OBJECTS)
	$(CC) $(D_CFLAGS) $(D_OBJECTS) $(LDFLAGS) -o $@

# DEBUG OBJECTS

$(OBJDIR)/main.d.o: $(SRCDIR)/main.cpp $(SRCDIR)/Error.h $(SRCDIR)/Assembler.h
	$(CC) -c $(D_CFLAGS) $(SRCDIR)/main.cpp -o $@ 

$(OBJDIR)/Assembler.d.o: $(SRCDIR)/Assembler.cpp $(SRCDIR)/Assembler.h $(SRCDIR)/Opcodes.h
	$(CC) -c $(D_CFLAGS) $(SRCDIR)/Assembler.cpp -o $@ 

$(OBJDIR)/Error.d.o: $(SRCDIR)/Error.cpp $(SRCDIR)/Error.h
	$(CC) -c $(D_CFLAGS) $(SRCDIR)/Error.cpp -o $@ 

$(OBJDIR)/crc.d.o: $(SRCDIR)/crc.c $(SRCDIR)/crc.h
	$(CC) -c $(D_CFLAGS) $(SRCDIR)/crc.c -o $@ 

#####################################################################
# ALL TARGETS

all: $(OBJECTS) tchip16 tchip16_debug 

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	-@test -d $(OBJDIR) || mkdir $(OBJDIR)

# CLEAN TARGET

clean:
	-@rm tchip16 tchip16_debug 2> /dev/null || true
	-@rm -rf $(OBJDIR)/ 2> /dev/null || true

# (UN)INSTALL TARGET

install:
ifneq ($(USER),root)
	-@echo You are not root, please run: \'sudo make install\'
else
	-@echo Installing binaries ...
	-@cp tchip16 tchip16_debug /usr/bin
	-@echo Installing man page ... 
	-@cp $(SRCDIR)/tchip16.1.gz /usr/share/man/man1/tchip16.1.gz
	-@echo Updating man-db ...
	-@mandb -q
	-@echo done.
endif

uninstall:
ifneq ($(USER),root)
	-@echo You are not root, please run: \'sudo make uninstall\'
else
	-@echo Removing binaries ...
	-@rm /usr/bin/tchip16 /usr/bin/tchip16_debug 2> /dev/null || true
	-@echo Removing man page ...
	-@rm /usr/share/man/man1/tchip16.1.gz 2> /dev/null || true
	-@echo Updating man-db ... 
	-@mandb -q 
	-@echo done.
endif
