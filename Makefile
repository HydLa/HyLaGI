main_projects := core 
lib_projects  := parser math_source_converter constraint_hierarchy simulator symbolic_simulator branch_and_prune_simulator
test_projects := unit_tests

projects := $(main_projects) $(test_projects) $(lib_projects)

.PHONY : all
all: $(main_projects)

# execute unit test
.PHONY : check
check: $(projects)
	cd unit_tests && ./unit_test

# ï∂éöÉRÅ[ÉhÇÃê›íË & propsetÇÃìoò^
charset:
	@sources=`find . -name "Makefile" -or -name "*.cpp" -or -name "*.h" -or -name "*.m"` && \
	echo $${sources} && \
	nkf -s -Lw --overwrite $$sources  && \
	svn propset svn:mime-type 'text/plain; charset=shift-jis' $$sources


# remove all temp files
.PHONY : clean
clean:
	@for i in $(projects); do \
	   (cd $$i && $(MAKE) clean); \
	done

# 
.PHONY : $(projects)
$(projects):
	$(MAKE) --directory=$@

# dependency
$(test_projects): $(lib_projects)
core: $(lib_projects)
symbolic_simulator: math_source_converter
