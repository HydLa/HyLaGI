TARGET	 := unit_test

LIB_DIRS := symbolic_simulator simulator constraint_hierarchy parser common virtual_constraint_solver # branch_and_prune_simulator  

INCLUDES += -I. $(addprefix -I../src/,$(LIB_DIRS))
#INCLUDES += -I../librealpaver -I../librealpaverbasic
INCLUDES += -I/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
INCLUDES += -I/usr/local/include/boost

LDFLAGS  += -L. $(addprefix -L../src/,$(LIB_DIRS))
#LDFLAGS  += -L../librealpaver  -L../librealpaverbasic
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/8.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
LDFLAGS	 += -L/usr/local/lib

LIBS     += $(addprefix -lhydla_,$(LIB_DIRS)) $(addprefix -lhydla_,$(LIB_DIRS)) #2回やって順序による依存関係の問題を解消
#LIBS     += -lrealpaver -lrealpaverbasic  
LIBS     += -lboost_unit_test_framework -lboost_regex 
#LIBS     += -lML32i3 -lm -lrt
LIBS     += -lML64i3 -lm -lrt

DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 

DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../src/$(dir)/libhydla_$(dir).a)

include ../src/Makefile.common
