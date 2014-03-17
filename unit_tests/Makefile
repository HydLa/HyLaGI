TARGET	 := unit_test

SRC_DIRS := ../src


LIB_DIRS := simulator constraint_hierarchy parser common backend
INCLUDE_DIRS := $(LIB_DIRS) simulator/symbolic_simulator

INCLUDES += -I. $(addprefix -I$(SRC_DIRS)/,$(INCLUDE_DIRS))
INCLUDES += -I$(SRC_DIRS)/backend/reduce/sexp/
INCLUDES += -I/usr/local/Wolfram/Mathematica/9.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
INCLUDES += -I/usr/local/include/boost

LDFLAGS  += -L. $(addprefix -L$(SRC_DIRS)/,$(LIB_DIRS))
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/9.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
LDFLAGS	 += -L/usr/local/lib

LIBS     += $(addprefix -lhydla_,$(LIB_DIRS)) $(addprefix -lhydla_,$(LIB_DIRS)) #2回やって順序による依存関係の問題を解消
LIBS     += -lboost_unit_test_framework -lboost_regex 
LIBS     += -lML64i3 -lm -lrt

DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 

DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),$(SRC_DIRS)/$(dir)/libhydla_$(dir).a)

include $(SRC_DIRS)/Makefile.common