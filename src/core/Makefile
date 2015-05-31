TARGET	 := ../../bin/hylagi

LIB_DIRS := backend simulator hierarchy parser common io interval symbolic_expression utility

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I/usr/local/include/boost
INCLUDES +=  -I../backend/mathematica
INCLUDES += -I../simulator/symbolic_simulator -I../simulator/hybrid_automata -I../parser/error

LDFLAGS  += -L. $(addprefix -L../,$(LIB_DIRS))
LDFLAGS	 += -L/usr/local/lib

LIBS     += $(addprefix -lhydla_,$(LIB_DIRS)) $(addprefix -lhydla_,$(LIB_DIRS)) #2回やって順序による依存関係の問題を解消

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
LIBS_STATIC += -lboost_program_options
LIBS     += -lML64i3 -lm -lrt
#LIBS     += -lML32i3 -lm -lrt
endif
ifeq ($(UNAME),Darwin)
LIBS_STATIC += -lboost_program_options-mt
LIBS     += -lMLi3 -lm -lstdc++.6.0.9
endif

CXX_OPT  += 



DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a)

include ../Makefile.common
