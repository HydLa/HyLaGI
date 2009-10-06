TARGET	 := ../hydla

INCLUDES += -I. -I../parser -I../symbolic_simulator
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
INCLUDES += -I/usr/local/include/boost-1_38

LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/lib -L../parser -L../symbolic_simulator

LIBS	 += -lhydla_symbolic_simulator -lhydla_parser -lboost_program_options
LIBS	 += -lML32i3 -lm -lrt
DEFINES  += 
CXX_OPT  += 

DEP_OBJECTS  := ../parser/libhydla_parser.a ../symbolic_simulator/libhydla_symbolic_simulator.a
DEP_PROJECTS := $(dir $(DEP_OBJECTS))

include ../Makefile.common

