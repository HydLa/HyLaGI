
SUBDIR := $(dir $(shell find . -mindepth 2 -name "Makefile"))

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

