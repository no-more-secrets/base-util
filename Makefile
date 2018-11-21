# This Makefile is just for convenience

configure-marker := build/CMakeFiles

all: $(configure-marker)
	@cd build && $(MAKE) -s all

run: $(configure-marker)
	@cd build && $(MAKE) -s run

install: $(configure-marker)
	@cd build && $(MAKE) -s install

clean:
	@cd build && $(MAKE) -s clean

test: all
	@build/test/tests

distclean:
	@rm -rf build

.PHONY: all clean run test distclean

$(configure-marker):
	@echo "You must run scripts/configure first!"
	@false
