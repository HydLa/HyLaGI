src_directory := src
UNAME := $(shell uname)
.PHONY : all
all: $(src_directory)
	make math-check
ifeq ($(UNAME),Linux)
	@mkdir bin -p && cd src && $(MAKE)
endif
ifeq ($(UNAME),Darwin)
	@mkdir -p bin && cd src && $(MAKE)
endif
##	@cd $(src_directory) $(MAKE)

# execute unit test
.PHONY : check
check: $(src_directory)
	cd unit_tests && $(MAKE) && ./unit_test

# execute unit mathematica test
.PHONY : math-check
math-check: $(src_directory)
	cd unit_math_tests && ./unit_math_test.sh

.PHONY : clean
clean:
	@cd $(src_directory) && $(MAKE) clean

# generate documentation
.PHONY : doc
doc:
	doxygen doc/doxygen.conf	

# remove documentation
.PHONY : doc-clean
doc-clean:
	rm -fvr doc/html
