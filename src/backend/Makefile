MAKE_TYPE := library

TARGET	 := libhydla_backend.a
INCLUDES += -I. -I./mathematica
INCLUDES += -I../parser -I../hierarchy -I../simulator -I../common -I../simulator/symbolic_simulator -I../symbolic_expression -I../parser/error -I../utility -I../interval
CXX_OPT  += 

SRC_DIR += mathematica

SRC         += mathematica/math_source.cpp
CLEAN_FILES += mathematica/math_source.cpp

include ../Makefile.common

mathematica/math_source.cpp: mathematica/math_source/load_first/*.m mathematica/math_source/*.m
	cat mathematica/math_source/load_first/*.m mathematica/math_source/*.m | \
		../math_source_converter/msc math_source > \
		mathematica/math_source.cpp
