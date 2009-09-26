MAKE_TYPE := library

TARGET	 := libhydla_symbolic_simulator.a
INCLUDES += -I. -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
INCLUDES += -I/usr/local/include/boost-1_38
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/lib
LIBS	 += -lML32i3 -lm -lrt
DEFINES  += 
CXX_OPT  += 

SRC         += math_source.cpp
CLEAN_FILES += math_source.cpp

include ../Makefile.common

../math_source_converter/msc:
	cd ../math_source_converter && $(MAKE) all

math_source.cpp: HydLa.m ../math_source_converter/msc
	 cat HydLa.m | ../math_source_converter/msc > math_source.cpp
