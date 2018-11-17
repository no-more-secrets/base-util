# This Makefile is just for convenience

all: build/CMakeFiles
	@cd build && $(MAKE) -s

clean:
	@cd build && $(MAKE) -s clean

test: all
	@build/tests/tests-all

run: build/CMakeFiles
	@cd build && $(MAKE) -s run

.PHONY: all clean test run

build/CMakeFiles:
	@scripts/configure
