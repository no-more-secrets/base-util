# This Makefile is just for convenience

.DEFAULT_GOAL := all

configure-marker := build/CMakeFiles

test: all
	@build/test/tests-all

.PHONY: test

# Just forwards targets
%: $(configure-marker)
	@cd build && $(MAKE) -s $@

$(configure-marker):
	@scripts/configure
