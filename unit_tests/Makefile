TARGET	 := unit_test

INCLUDES += -I. -I../ -I../constraint_hierarchy -I../parser -I../simulator -I../symbolic_simulator
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/

LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L../parser -L../simulator -L../constraint_hierarchy -L../symbolic_simulator

LIBS	 += -lhydla_simulator -lhydla_constraint_hierarchy -lhydla_symbolic_simulator -lhydla_parser -lboost_unit_test_framework -lboost_regex
LIBS	 += -lML32i3 -lm -lrt
DEFINES  += 
CXX_OPT  += 

DEP_OBJECTS  := ../parser/libhydla_parser.a ../symbolic_simulator/libhydla_symbolic_simulator.a

include ../Makefile.common

