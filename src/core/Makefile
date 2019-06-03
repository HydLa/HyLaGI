TARGET	 := ../../bin/hylagi

LIB_DIRS := backend simulator hierarchy parser common io symbolic_expression utility interval

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I/usr/local/include/boost
INCLUDES +=  -I../backend/mathematica
INCLUDES += -I../simulator/symbolic_simulator -I../simulator/hybrid_automata -I../parser/error

LDFLAGS  += -L. $(addprefix -L../,$(LIB_DIRS))
LDFLAGS	 += -L/usr/local/lib

LIBS     += $(addprefix -lhydla_,$(LIB_DIRS)) $(addprefix -lhydla_,$(LIB_DIRS)) #2回やって順序による依存関係の問題を解消

ARCH := $(shell getconf LONG_BIT)

MATH_FLAGS_32 := -lWSTP32i4
MATH_FLAGS_64 := -lWSTP64i4

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
LIBS_STATIC += -lboost_program_options
LIBS     += $(MATH_FLAGS_$(ARCH)) -lm -lrt
endif
ifeq ($(UNAME),Darwin)
LIBS_STATIC += -lboost_program_options-mt
LIBS     += -lWSTPi4 -lm -lstdc++.6.0.9
endif

CXX_OPT  += 



DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a)

include ../Makefile.common
