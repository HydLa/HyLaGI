MAKE_TYPE := library

TARGET	 := libhydla_symbolic_legacy_simulator.a
INCLUDES += -I. -I../parser -I../common
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
DEFINES  += 
CXX_OPT  += 

SRC         += sls_math_source.cpp
CLEAN_FILES += sls_math_source.cpp

include ../Makefile.common

sls_math_source.cpp: sls_math_source.m
	cat sls_math_source.m | \
		../math_source_converter/msc sls_math_source >\
		sls_math_source.cpp
