src_directory := src
.PHONY : all
all: $(src_directory)
	@mkdir bin -p && cd src && make
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
