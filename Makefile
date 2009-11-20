main_projects := parser math_source_converter constraint_hierarchy symbolic_simulator branch_and_prune_simulator core simulator
test_projects := unit_tests

projects := $(main_projects) $(test_projects)

.PHONY : all
all: $(main_projects)

# execute unit test
.PHONY : check
check: $(projects)
	cd unit_tests && ./unit_test

# 文字コードの設定 & propsetの登録
charset:
	@sources=`find . -name "Makefile" -or -name "*.cpp" -or -name "*.h" -or -name "*.m"` && \
	echo $${sources} && \
	nkf -w8 -Lw --overwrite $$sources  && \
	svn propset svn:mime-type 'text/plain; charset=utf-8' $$sources


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
$(test_projects): parser symbolic_simulator constraint_hierarchy branch_and_prune_simulator simulator
core: parser symbolic_simulator constraint_hierarchy branch_and_prune_simulator simulator
symbolic_simulator: math_source_converter
