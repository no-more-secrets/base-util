.DEFAULT_GOAL := all
builds        := .builds
build-current := .builds/current

-include $(build-current)/env-vars.mk

possible_targets := all clean

build-config := $(notdir $(realpath $(build-current)))
ifneq (,$(wildcard $(build-current)/Makefile))
    # Here we are invoking $(MAKE) directly instead of using
    # cmake because otherwise there seem to be issues with
    # propagating the jobserver.  For this same reason we
    # also do not just put the whole command into a variable
    # and just define the targets once.
    $(possible_targets): $(build-current)
	    @cd $(build-current) && $(MAKE) -s $@
else
    # Use cmake to build here because it is the preferred
    # way to go when it works for us (which it does in this
    # case).
    $(possible_targets): $(build-current)
	    @cd $(build-current) && cmake --build . --target $@
endif

run: all
	@$(build-current)/app/main

test: all
	@$(build-current)/test/tests

clean-target := $(if $(wildcard $(builds)),clean,)

# Need to have `clean` as a dependency before removing the
# .builds folder because some outputs of the build are in the
# source tree and we need to clear them first.
distclean: $(clean-target)
	@rm -rf .builds

update:
	@git pull origin master
	@git submodule update --init
	@cmc rc
	@echo
	@$(MAKE) -s test

$(build-current):
	@cmc

.PHONY: $(possible_targets) update run test
