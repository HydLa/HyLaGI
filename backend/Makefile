MAKE_TYPE := library

TARGET	 := libhydla_backend.a
INCLUDES += -I. -I./mathematica -I./reduce -I./reduce/sexp
INCLUDES += -I../parser -I../constraint_hierarchy -I../simulator -I../common -I../symbolic_simulator
INCLUDES += -I/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 

SRC_DIR += mathematica #reduce reduce/sexp

SRC         += mathematica/math_source.cpp
#SRC         += reduce/vcs_reduce_source.cpp
CLEAN_FILES += mathematica/math_source.cpp
#CLEAN_FILES += reduce/vcs_reduce_source.cpp

include ../Makefile.common

mathematica/math_source.cpp: mathematica/math_source/*.m
	cat mathematica/math_source/*.m | \
		../math_source_converter/msc math_source > \
		mathematica/math_source.cpp

#reduce/vcs_reduce_source.cpp: reduce/vcs_reduce_source.red
#	cat reduce/vcs_reduce_source.red | \
#		../reduce_source_converter/rsc vcs_reduce_source > \
#		reduce/vcs_reduce_source.cpp