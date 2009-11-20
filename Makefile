main_projects := parser math_source_converter constraint_hierarchy symbolic_simulator core simulator
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
	@sources=`find . -name "*.cpp" -or -name "*.h"` && \
	echo $${sources} && \
	nkf -s -Lw --overwrite $$sources && \
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
$(test_projects): parser symbolic_simulator constraint_hierarchy simulator
core: parser symbolic_simulator constraint_hierarchy simulator
symbolic_simulator: math_source_converter
