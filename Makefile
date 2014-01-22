src_directory := src

.PHONY : all
all: $(src_directory)
	@mkdir bin -p && cd src && make
##	@cd $(src_directory) make

.PHONY : clean
clean:
	@cd $(src_directory) make clean
