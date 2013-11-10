MAKE_TYPE := library

TARGET	 := libhydla_symbolic_simulator.a
INCLUDES += -I. -I../parser -I../constraint_hierarchy -I../simulator -I../common -I../backend -I../backend/mathematica -I../backend/reduce
INCLUDES += -I/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LIBS	 += -lML64i3 -lm -lrt
#LIBS	 += -lML32i3 -lm -lrt
DEFINES  += 
CXX_OPT  += 

include ../Makefile.common
