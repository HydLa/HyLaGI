MAKE_TYPE := library

TARGET	 := libhydla_virtual_constraint_solver.a
INCLUDES += -I. -I./mathematica -I./realpaver -I./reduce
INCLUDES += -I../parser -I../constraint_hierarchy -I../simulator -I../common -I../librealpaver -I../librealpaverbasic
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 

SRC_DIR += mathematica realpaver reduce

SRC         += mathematica/vcs_math_source.cpp
SRC         += reduce/vcs_reduce_source.cpp
CLEAN_FILES += mathematica/vcs_math_source.cpp
CLEAN_FILES += reduce/vcs_reduce_source.cpp

include ../Makefile.common

mathematica/vcs_math_source.cpp: mathematica/vcs_math_source.m
	cat mathematica/vcs_math_source.m | \
		../math_source_converter/msc vcs_math_source > \
		mathematica/vcs_math_source.cpp

reduce/vcs_reduce_source.cpp: reduce/vcs_reduce_source.red
	cat reduce/vcs_reduce_source.red | \
		../reduce_source_converter/rsc vcs_reduce_source > \
		reduce/vcs_reduce_source.cpp

