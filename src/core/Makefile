TARGET	 := ../../bin/hylagi

LIB_DIRS := backend simulator hierarchy parser common io symbolic_expression utility interval

INCLUDES += -I. $(addprefix -I../,$(LIB_DIRS))
INCLUDES += -I/usr/local/include/boost
INCLUDES +=  -I../backend/mathematica
INCLUDES += -I../simulator/symbolic_simulator -I../simulator/hybrid_automata -I../parser/error -I..parser/never_claim

LDFLAGS  += -L. $(addprefix -L../,$(LIB_DIRS))
LDFLAGS  += -L../parser/never_claim
LDFLAGS	 += -L/usr/local/lib

LIBS     += ../parser/never_claim/libhydla_nc_parser.a $(addprefix -lhydla_,$(LIB_DIRS)) ../parser/never_claim/libhydla_nc_parser.a $(addprefix -lhydla_,$(LIB_DIRS)) #2回やって順序による依存関係の問題を解消

ARCH := $(shell getconf LONG_BIT)

MATH_FLAGS_32 := -lML32i3
MATH_FLAGS_64 := -lML64i3

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
LIBS_STATIC += -lboost_program_options
LIBS     += $(MATH_FLAGS_$(ARCH)) -lm -lrt
endif
ifeq ($(UNAME),Darwin)
LIBS_STATIC += -lboost_program_options-mt
LIBS     += -lMLi3 -lm -lstdc++.6.0.9
endif

CXX_OPT  += 


# ad hoc modification for never claim parser by betsuno
DEP_OBJECTS  := $(foreach dir,$(LIB_DIRS),../$(dir)/libhydla_$(dir).a) ../parser/never_claim/libhydla_nc_parser.a

include ../Makefile.common
