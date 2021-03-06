src_directory := src
UNAME := $(shell uname)
.PHONY : all
all: $(src_directory)
	@make math-check
	@mkdir -p bin && cd src && $(MAKE)
	@printf "%s \033[32m%s\033[m\n" "build" "succeeded"

# execute system test
.PHONY : test
test: $(src_directory)
	@cd system_test && ./system_test.sh

# execute unit test
.PHONY : check
check: $(src_directory)
	@cd unit_tests && $(MAKE) && ./unit_test

# execute unit mathematica test
.PHONY : math-check
math-check: $(src_directory)
	@cd unit_math_tests && ./unit_math_test.sh

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
