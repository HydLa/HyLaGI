MAKE_TYPE := library

TARGET	 := libhydla_branch_and_prune_simulator.a
INCLUDES += -I. -I../parser -I../constraint_hierarchy -I../simulator -I../librealpaver -I../librealpaverbasic
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LIBS	 += -lML32i3 -lm -lrt
DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 

include ../Makefile.common
