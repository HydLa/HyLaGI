TARGET	 := ../hydla

LIB_DIRS := symbolic_simulator symbolic_legacy_simulator branch_and_prune_simulator virtual_constraint_solver simulator constraint_hierarchy parser common

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I../librealpaver -I../librealpaverbasic
INCLUDES += -I/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
INCLUDES += -I/usr/local/include/boost-1_38

LDFLAGS  += -L. $(addprefix -L../,$(LIB_DIRS))
LDFLAGS  += -L../librealpaver  -L../librealpaverbasic
LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/7.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
LDFLAGS	 += -L/usr/local/lib


LIBS     += $(addprefix -lhydla_,$(LIB_DIRS)) $(addprefix -lhydla_,$(LIB_DIRS)) #2âÒÇ‚Ç¡ÇƒèáèòÇ…ÇÊÇÈàÀë∂ä÷åWÇÃñ‚ëËÇâè¡
LIBS     += -lrealpaver -lrealpaverbasic  
LIBS     += -lboost_program_options
LIBS     += -lML64i3 -lm -lrt

DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 



DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a)

include ../Makefile.common