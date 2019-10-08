MAKE_TYPE := library

TARGET	 := libhydla_simulator.a
INCLUDES += -I. -I../parser -I../hierarchy -I../common -I./symbolic_simulator -I./hybrid_automata -I../io -I../solver -I../backend -I../backend/mathematica -I../backend/reduce -I../interval -I../symbolic_expression -I../utility -I../parser/error
LIBS	 += -lWSTP64i4 -lmaplec -lm -lrt
DEFINES  += 
CXX_OPT  += 

SRC_DIR += symbolic_simulator hybrid_automata

include ../Makefile.common
