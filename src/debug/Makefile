MAKE_TYPE := library

TARGET	 := libhydla_debug.a
INCLUDES += -I. -I../parser -I../simulator -I../common -I../hierarchy -I../simulator/symbolic_simulator -I../backend -I../backend/mathematica -I../symbolic_expression -I../parser/error -I../backend/reduce -I/usr/include/python2.7 -L/usr/lib/python2.7/config -lpython2.7 -I../io
LIBS	 += -lML64i3 -lm -lrt
DEFINES  += 
CXX_OPT  += -DAFFINE_SIMPLE=1

include ../Makefile.common

