TARGET	 := ../hyrose

LIB_DIRS := symbolic_simulator backend simulator constraint_hierarchy parser common output interval

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I/usr/local/include/boost
INCLUDES +=  -I../backend/mathematica -I../backend/reduce

LDFLAGS  += -L. $(addprefix -L../,$(LIB_DIRS))
LDFLAGS	 += -L/usr/local/lib

LIBS     += $(addprefix -lhydla_,$(LIB_DIRS)) $(addprefix -lhydla_,$(LIB_DIRS)) #2回やって順序による依存関係の問題を解消
#LIBS     += -lrealpaver -lrealpaverbasic  
LIBS     += -lboost_program_options
LIBS     += -lML64i3 -lm -lrt
#LIBS     += -lML32i3 -lm -lrt

DEFINES  += -DRP_SYSTEM_LINUX_IX86
CXX_OPT  += 



DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a)

include ../Makefile.common
