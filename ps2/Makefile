export PATH := /home/shay/a/ece270/bin:/usr/bin:$(PATH)
export LD_LIBRARY_PATH := /home/shay/a/ece270/lib:/usr/lib:$(LD_LIBRARY_PATH)
modules := controller

all:
	@for mod in $(modules); do \
		make test_$$mod; \
		echo; \
	done

test_%: %.sv
	@echo "\033[0;33m============================== $@ ==============================\033[0m"
	@rm -rf $*_dir
	@echo Compiling $*...
	@echo "You'll see a LOT of text printed out once the test runs.  Don't be alarmed.  Find red FAILed tests."
	@verilator --cc --build --exe -Wno-CASEINCOMPLETE --Mdir $*_dir $*.sv ~ece270/etc/project/tests/$*.cpp 1>/dev/null
	@echo Testing $*...
	@if $*_dir/V$*; then \
			echo "\033[0;32m============================== TEST PASSED ==============================\033[0m"; \
	else \
			echo "\033[0;31m============================== TEST FAILED ==============================\033[0m"; \
	fi
	@rm -rf $*_dir
