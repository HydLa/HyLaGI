MAKE_TYPE := library

TARGET	 := libhydla_nc_parser.a
INCLUDES += -I. -I../ -I../../common -I../error -I../../symbolic_expression -I../../utility -I../../simulator -I../../hierarchy
INCLUDES += -I/usr/local/include/boost
LDFLAGS	 += -L/usr/local/lib
LIBS	 += 
DEFINES  += 
CXX_OPT  +=

include ../../Makefile.common

all: nc_parser

nc_parser: y.tab.cpp lex.yy.o
	$(CXX) $(INCLUDES) $(LDFLAGS) $(CXX_OPT) -c $^ -o $(BIN).o 

lex.yy.o: lex.yy.c y.tab.cpp 
	$(CXX) $(INCLUDES) -c lex.yy.c -o $@

y.tab.cpp: nc.y
	yacc --verbose --debug -d $< -o $@

lex.yy.c: nc.l
	flex $<

clean:
	$(RM) -f *.a *.d *.o y.output y.tab.hpp y.tab.h lex.yy.c y.tab.cpp y.tab.c $(BIN)

libhydla_nc_parser.a: 

# # common =========================================
# .SUFFIXES: 
# .SUFFIXES: .o .c .cc .cpp .h .d
# 
# CC       := clang
# CXX      := clang++
# SED      := sed #@SED@
# MV       := mv #@MV@
# AR       := ar #@AR@
# RM	 := rm #@RM@
# LN       := ln #@LN@
# 
# LIBS += -lpthread
# 
# UNAME := $(shell uname)
# 
# ifeq ($(UNAME),Darwin)
# LIBS_STATIC += -lboost_thread-mt -lboost_regex-mt
# else
# LIBS_STATIC += -lboost_thread-mt -lboost_regex
# endif
# 
# ifeq ($(UNAME),Linux)
# INCLUDES += -I/usr/local/Wolfram/Mathematica/9.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
# LDFLAGS	 += -L/usr/local/Wolfram/Mathematica/9.0/SystemFiles/Links/MathLink/DeveloperKit/Linux-x86-64/CompilerAdditions/
# 
# CXX_OPT  += -Wall -std=c++11 -O
# # CXX_OPT  += -Wall --input-charset=cp932 -O2 -pg
# endif
# ifeq ($(UNAME),Darwin)
# INCLUDES += -I/Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit/MacOSX-x86-64/CompilerAdditions/
# LDFLAGS  += -L/Applications/Mathematica.app/SystemFiles/Links/MathLink/DeveloperKit/MacOSX-x86-64/CompilerAdditions/
# 
# CXX_OPT  += -Wall -std=c++11 -framework Foundation
# endif
# 
# ARFLAGS  := cru
# CXXFLAGS += $(INCLUDES) $(DEFINES) $(CXX_OPT)
# 
# SRC_DIR  += .
# SRC      += $(foreach dir,$(SRC_DIR), $(wildcard $(dir)/*.c) $(wildcard $(dir)/*.cc) $(wildcard $(dir)/*.cpp)) 
# OBJECT   := $(subst .c,.o,$(filter %.c,$(SRC))) \
# 	    $(subst .cc,.o,$(filter %.cc,$(SRC))) \
# 	    $(subst .cpp,.o,$(filter %.cpp,$(SRC)))
# DEP_FILE := $(OBJECT:.o=.d)
# 
# .PHONY : all
# all: $(DEP_PROJECTS) $(TARGET)
# 
# .PHONY : clean
# clean:
# 	-$(RM) -fr $(TARGET) $(OBJECT) $(DEP_FILE) $(CLEAN_FILES)
# 
# $(TARGET): $(DEP_OBJECTS) $(OBJECT)
# ifeq ($(MAKE_TYPE),library)
# 	$(AR) $(ARFLAGS) $@ $(OBJECT)
# 	ranlib $@
# else ifeq ($(UNAME),Darwin)
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECT) $(LIBS) $(LIBS_STATIC)
# else
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECT) -Wl,-Bstatic $(LIBS_STATIC) -Wl,-Bdynamic $(LIBS)
# endif
# 
# #$(DEP_OBJECTS):
# #	$(MAKE) --directory=$(dir $@)
# 
# ifneq "$(MAKECMDGOALS)" "clean"
# -include $(DEP_FILE)
# endif
# 
# .c.o:
# 	$(CC) $(CXXFLAGS) -c -o $@ $<
# 
# .cc.o .cpp.o: 
# 	$(CXX) $(CXXFLAGS) -c -o $@ $<
# 
# .c.d .cc.d .cpp.d:
# 	$(CXX) $(CXXFLAGS) $(TARGET_ARCH) -M $< | \
# 	$(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: $(MAKE_INC) ,' >$@.tmp
# 	$(MV) $@.tmp $@
# 
# check-syntax:
# 	$(CXX) -o nul $(CXXFLAGS) -S ${CHK_SOURCES}
# common =========================================

#  CC = clang 
#  CXX = clang++ $(CXXFLAGS)
#  CXXFLAGS = -Wall -std=c++14 $(LDFLAGS) $(INCLUDE_FLAGS)
#  LDFLAGS = -L/usr/local/lib 
#  BIN = nc_parser
#  TARGET	 := libhydla_nc_parser.a
#  
#  HYLAGI_SRC = /home/betsuno/HyLaGI/src
#  INCLUDE_FLAGS = -I /usr/local/include -I $(HYLAGI_SRC)/simulator -I $(HYLAGI_SRC)/hierarchy -I $(HYLAGI_SRC)/symbolic_expression
