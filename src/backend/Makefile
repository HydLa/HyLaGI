MAKE_TYPE := library

TARGET	 := libhydla_backend.a
INCLUDES += -I. -I./mathematica -I./reduce -I./reduce/sexp
INCLUDES += -I../parser -I../hierarchy -I../simulator -I../common -I../simulator/symbolic_simulator -I../symbolic_expression -I../parser/error
DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 

SRC_DIR += mathematica reduce reduce/sexp

SRC         += mathematica/math_source.cpp
SRC         += reduce/reduce_source.cpp
CLEAN_FILES += mathematica/math_source.cpp
CLEAN_FILES += reduce/reduce_source.cpp

include ../Makefile.common

mathematica/math_source.cpp: mathematica/math_source/must_be_first/*.m mathematica/math_source/*.m
	cat mathematica/math_source/must_be_first/*.m mathematica/math_source/*.m | \
		../math_source_converter/msc math_source > \
		mathematica/math_source.cpp

reduce/reduce_source.cpp: reduce/reduce_source.red
	cat reduce/reduce_source.red | \
		../reduce_source_converter/rsc reduce_source > \
		reduce/reduce_source.cpp
