MAKE_TYPE := library

TARGET	 := libhydla_symbolic_simulator.a
INCLUDES += -I. -I../parser -I../constraint_hierarchy -I../simulator -I../common
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LIBS	 += -lML32i3 -lm -lrt
DEFINES  += 
CXX_OPT  += 

SRC         += math_source.cpp
CLEAN_FILES += math_source.cpp

#DEP_OBJECTS  := ../math_source_converter/msc

include ../Makefile.common

math_source.cpp: HydLa.m
	cat HydLa.m | ../math_source_converter/msc > math_source.cpp
