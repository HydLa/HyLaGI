main_projects := core 
lib_projects  := common parser math_source_converter reduce_source_converter hierarchy simulator backend io symbolic_expression utility
test_projects := unit_tests

projects := $(main_projects) $(test_projects) $(lib_projects)

.PHONY : all
all: $(main_projects)

# 文字コードの設定 & propsetの登録
charset:
	@sources=`find . -name "Makefile" -or -name "*.cpp" -or -name "*.h" -or -name "*.m"` -or -name "*.red"` && \
	echo $${sources} && \
	nkf -w -Lu --overwrite $$sources  && \
	svn propset svn:mime-type 'text/plain; charset=utf-8' $$sources

charset_guess:
	@nkf --guess `find . -name "*.cpp" -or -name "*.h" `


#latexを出力する場合
#	nkf -e --overwrite `find doc/latex -name "*.tex"`
#	make -f doc/latex
#	dvipdfmx doc/latex/refman.dvi
#	cp doc/latex/refman.dvi doc/

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
backend: math_source_converter reduce_source_converter
