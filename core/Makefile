TARGET	 := ../hydla

LIB_DIRS := symbolic_simulator branch_and_prune_simulator simulator constraint_hierarchy parser common

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I../librealpaver -I../librealpaverbasic
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
INCLUDES += -I/usr/local/include/boost-1_38

LDFLAGS  += -L. $(addprefix -L../,$(LIB_DIRS))
LDFLAGS  += -L../librealpaver  -L../librealpaverbasic
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux/CompilerAdditions/
LDFLAGS	 += -L/usr/local/lib

LIBS     += $(addprefix -lhydla_,$(LIB_DIRS)) $(addprefix -lhydla_,$(LIB_DIRS)) #2回やって順序による依存関係の問題を解消
LIBS     += -lrealpaver -lrealpaverbasic  
LIBS     += -lboost_program_options
LIBS     += -lML32i3 -lm -lrt

DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 



DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a)

include ../Makefile.common

