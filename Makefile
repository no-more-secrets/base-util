.DEFAULT_GOAL := all
build-current := .builds/current

all run clean: $(build-current)
	@cd $(build-current) && $(MAKE) -s $@

test: all
	@.builds/current/test/tests

distclean:
	@rm -rf .builds

$(build-current):
	@scripts/configure

.PHONY: all run clean test distclean
