TARGET	 := ../../bin/hylagi

LIB_DIRS := backend simulator hierarchy parser common io interval debug symbolic_expression utility

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I/usr/local/include/boost
INCLUDES +=  -I../backend/mathematica
INCLUDES += -I../simulator/symbolic_simulator -I../simulator/hybrid_automata -I../parser/error
INCLUDES += `python3-config --includes --ldflags`

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
LIBS     += -lWSTPi4 -lm
endif

CXX_OPT  += 

DEFINES += -DCOMMIT_ID=\"$(shell git show -s --format="%h")\" -DBRANCH_NAME=\"$(shell git rev-parse --abbrev-ref HEAD)\"

DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a)

include ../Makefile.common
