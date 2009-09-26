
SUBDIR := parser math_source_converter symbolic_simulator core
#$(dir $(shell find . -mindepth 2 -name "Makefile"))

.PHONY : all
all:
	@for i in $(SUBDIR); do \
	   (cd $$i && $(MAKE) all); \
	done

.PHONY : clean
clean:
	@for i in $(SUBDIR); do \
	   (cd $$i && $(MAKE) clean); \
	done
	-$(RM) -fr Debug Release *.user *.ncb

