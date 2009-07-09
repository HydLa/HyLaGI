.SUFFIXES: 
.SUFFIXES: .o .c .cc .cpp .h .d

MAKE_INC := Makefile.inc
include $(MAKE_INC)

CXX      := g++
SED      := sed #@SED@
MV       := mv #@MV@
AR       := ar #@AR@
RM	 := rm #@RM@
LN       := ln #@LN@

ARFLAGS  := crus
CXXFLAGS := $(INCLUDES) $(DEFINES) $(CXX_OPT)

SRC      := $(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp)
OBJECT   := $(addsuffix .o, $(basename $(SRC)))
DEP_FILE := $(OBJECT:.o=.d)

.PHONY : all
all: $(TARGET)

.PHONY : clean
clean:
	-$(RM) -f $(TARGET) $(OBJECT) $(DEP_FILE)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEP_FILE)
endif

.c.o .cc.o .cpp.o: 
	$(CXX) $(CXXFLAGS) -c $<

.c.d .cc.d .cpp.d:
	$(CXX) $(CXXFLAGS) $(TARGET_ARCH) -M $< | \
	$(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: $(MAKE_INC) ,' >$@.tmp
	$(MV) $@.tmp $@

$(TARGET): $(OBJECT)
ifeq ($(MAKE_TYPE), library)
	$(AR) $(ARFLAGS) $@ $(OBJECT)
else
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECT) $(LIBS)
endif
