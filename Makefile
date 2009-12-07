main_projects := core 
lib_projects  := parser math_source_converter constraint_hierarchy simulator symbolic_simulator librealpaver librealpaverbasic branch_and_prune_simulator
test_projects := unit_tests

projects := $(main_projects) $(test_projects) $(lib_projects)

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
	nkf -s -Lw --overwrite $$sources  && \
	svn propset svn:mime-type 'text/plain; charset=shift-jis' $$sources

charset_guess:
	@nkf --guess `find . -name "*.cpp" -or -name "*.h" `

# generate documentation
.PHONY : doc
doc:
	doxygen doc/doxygen.conf

# remove documentation
.PHONY : doc-clean
doc-clean:
	rm -fvr doc/html

# remove all temp files
.PHONY : clean
clean:
	@for i in $(projects); do \
	   (cd $$i && $(MAKE) clean); \
	done

.PHONY : distclean
distclean: doc-clean clean


# 
.PHONY : $(projects)
$(projects):
	$(MAKE) --directory=$@

# dependency
$(test_projects): $(lib_projects)
core: $(lib_projects)
symbolic_simulator: math_source_converter
branch_and_prune_simulator : librealpaver
librealpaver: librealpaverbasic
