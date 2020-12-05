MAKE_TYPE := library

TARGET	 := libhydla_debug.a
INCLUDES += -I. -I../parser -I../simulator -I../common -I../hierarchy -I../simulator/symbolic_simulator -I../backend -I../backend/mathematica -I../symbolic_expression -I../parser/error -I../backend/reduce `python3-config --includes --ldflags` -I../io
LIBS	 += -lWSTP64i4 -lm -lrt
DEFINES  += 
CXX_OPT  += -DAFFINE_SIMPLE=1

include ../Makefile.common

