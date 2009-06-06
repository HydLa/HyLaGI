.SUFFIXES: 
.SUFFIXES: .o .c .cc .cpp .h .d
.PHONY : clean

MAKE_INC := Makefile.inc
include $(MAKE_INC)

#SRC      = HydLa_parser/HydLaParser.cpp HydLa_parser/main.cpp
#MATH_DIR = 

CXX      := g++

SED      := sed #@SED@
MV       := mv #@MV@
AR       := ar #@AR@
RM	 := rm #@RM@
LN       := ln #@LN@

ARFLAGS  := crus
CFLAGS	 := $(INCLUDES) $(DEFINES) $(C_OPT)

SRC      := $(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp)
OBJECT   := $(foreach file, $(SRC), $(basename $(file)).o)
DEP_FILE := $(OBJECT:.o=.d)


all:
	@if [ ! -z $(TARGET) ]; then ${MAKE} $(TARGET); fi

#	g++ -I/home/hirose/boost_1_38_0 -o hydla_par $(SRC) -Wall -I${MATH_DIR} -L${MATH_DIR} 
#	g++ mathtest.cpp -Wall -I${MATH_DIR} -L${MATH_DIR} -lML32i3 -lm -lrt

clean:
	-$(RM) -f $(TARGET) $(OBJECT) $(DEP_FILE)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEP_FILE)
endif

.c.o .cc.o .cpp.o: 
	$(CXX) $(CFLAGS) -c $<

.c.d .cc.d .cpp.d:
	$(CXX) $(CFLAGS) $(TARGET_ARCH) -M $< | \
	$(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: $(MAKE_INC) $(MAKE_CFG) ,' >$@.tmp
	$(MV) $@.tmp $@

$(TARGET): $(OBJECT)
ifeq ($(MAKE_TYPE), library)
	$(AR) $(ARFLAGS) $@ $(OBJECT)
else
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECT) $(LIBS)
endif
