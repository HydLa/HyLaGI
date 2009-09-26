
SUBDIR := HydLa_parser math_source_converter
#$(dir $(shell find . -mindepth 2 -name "Makefile"))

.PHONY : all
all:
	(cd math_source_converter && $(MAKE)) && \
	 cat HydLa.m | ./msc > HydLa_parser/math_source.cpp;
	(cd HydLa_parser && $(MAKE));

# 	@for i in $(SUBDIR); do \
# 	   (cd $$i && $(MAKE) all); \
# 	done

.PHONY : clean
clean:
	(cd HydLa_parser && $(MAKE) clean)
	(cd math_source_converter && $(MAKE) clean)
	-$(RM) HydLa_parser/math_source.cpp
	-$(RM) -fr Debug Release *.user *.ncb

