MAKE_TYPE := library

TARGET	 := libhydla_simulator.a
INCLUDES += -I. -I../parser -I../constraint_hierarchy -I../common -I../symbolic_simulator -I../output -I../solver -I../backend -I../backend/mathematica -I../backend/reduce
INCLUDES += -I/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LIBS	 += -lML64i3 -lm -lrt
DEFINES  += 
CXX_OPT  += 

include ../Makefile.common
