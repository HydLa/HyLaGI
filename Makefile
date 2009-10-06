projects := parser math_source_converter symbolic_simulator core

.PHONY : all
all: $(projects)

.PHONY : clean
clean:
	@for i in $(projects); do \
	   (cd $$i && $(MAKE) clean); \
	done

.PHONY : $(projects)
$(projects):
	$(MAKE) --directory=$@

# dependency
core: parser symbolic_simulator
symbolic_simulator: math_source_converter
