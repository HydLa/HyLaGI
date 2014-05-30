src_directory := src
UNAME := $(shell uname)
.PHONY : all
all: $(src_directory)
ifeq ($(UNAME),Linux)
	@mkdir bin -p && cd src && make
endif
ifeq ($(UNAME),Darwin)
	@mkdir -p bin && cd src && make
endif
##	@cd $(src_directory) make

# execute unit test
.PHONY : check
check: $(src_directory)
	cd unit_tests && make && ./unit_test

.PHONY : clean
clean:
	@cd $(src_directory) && make clean

# generate documentation
.PHONY : doc
doc:
	doxygen doc/doxygen.conf	

# remove documentation
.PHONY : doc-clean
doc-clean:
	rm -fvr doc/html
