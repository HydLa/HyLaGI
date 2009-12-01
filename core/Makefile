TARGET	 := ../hydla

LIB_DIRS := parser constraint_hierarchy symbolic_simulator branch_and_prune_simulator simulator 

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
INCLUDES += -I/usr/local/include/boost-1_38

LDFLAGS  += -L. $(addprefix -L../,$(LIB_DIRS))
LDFLAGS  +=  -L../librealpaver  -L../librealpaverbasic
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/lib

LIBS     += $(addprefix -lhydla_,$(LIB_DIRS))
LIBS     += -lrealpaver -lrealpaverbasic
LIBS     += -lboost_program_options
LIBS     += -lML32i3 -lm -lrt

DEFINES  += 
CXX_OPT  += 

DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a)

include ../Makefile.common

