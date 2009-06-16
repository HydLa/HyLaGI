.SUFFIXES: 
.SUFFIXES: .o .c .cc .cpp .h .d
.PHONY : clean

all:
	cd HydLa_parser; ${MAKE}

clean:
	cd HydLa_parser; ${MAKE} clean

