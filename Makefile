main_projects := parser math_source_converter constraint_hierarchy symbolic_simulator core
test_projects := unit_tests

projects := $(main_projects) $(test_projects)

.PHONY : all
all: $(main_projects)

.PHONY : check
check: $(projects)
	cd unit_tests && ./unit_test

.PHONY : clean
clean:
	@for i in $(projects); do \
	   (cd $$i && $(MAKE) clean); \
	done

.PHONY : $(projects)
$(projects):
	$(MAKE) --directory=$@

# dependency
$(test_projects): parser symbolic_simulator constraint_hierarchy
core: parser symbolic_simulator constraint_hierarchy
symbolic_simulator: math_source_converter
