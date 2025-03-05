export PATH := /home/shay/a/ece270/bin:$(PATH)
export LD_LIBRARY_PATH := /home/shay/a/ece270/lib:$(LD_LIBRARY_PATH)
modules := simonsays
submodules := ../ps1/ssdec.sv ../ps1/mem.sv ../ps1/scankey.sv ../ps1/clkdiv.sv ../ps1/shiftdown.sv ../ps1/freerun.sv ../ps1/ssdec_ext.sv ../ps2/controller.sv

YOSYS=yosys
NEXTPNR=nextpnr-ice40
SHELL=bash

PROJ	= simonsays
PINMAP 	= pinmap.pcf
TCLPREF = addwave.gtkw
TB	= tb.sv
SRC	= top.sv
ICE   	= ice40hx8k.sv
UART	= support/uart.v support/uart_tx.v support/uart_rx.v
FILES   = $(ICE) $(SRC) $(UART) $(PROJ).sv $(submodules)
TRACE	= $(PROJ).vcd
BUILD   = ./build

DEVICE  = 8k
TIMEDEV = hx8k
FOOTPRINT = ct256

all:
	@for mod in $(modules); do \
		make test_$$mod; \
		echo; \
	done

repeated:
	@for i in 1 2 3 4 5 6 7 8 9 10; do \
		make; \
		sleep 0.25; \
	done

test_%: %.sv
	@echo "\033[0;33m============================== $@ ==============================\033[0m"
	@rm -f simonsays.vcd
	@echo Compiling $*...
	@echo "You'll see a LOT of text printed out once the test runs.  Don't be alarmed.  Find red FAILed tests."
	@echo "This version of the testbench will generate waveform traces so that you can debug your design.  See the project step info on how to use it."
	@verilator --cc --build --exe -Wno-CASEINCOMPLETE -Wno-PINMISSING --trace --timescale-override 1ms/1ms --top-module $* --Mdir $*_dir $(submodules) $*.sv ~ece270/etc/project/tests/$*.cpp 1>/dev/null
	@echo Testing $*...
	@if $*_dir/V$*; then \
			echo "\033[0;32m============================== TEST PASSED ==============================\033[0m"; \
			echo "You may open GTKwave to view the design timing traces anyway if you wish by running 'gtkwave --save simonsays.gtkw'."; \
			echo ""; \
			echo "This testbench generates randomized press.mem files.  You should run the 'make repeated' target to run this 10 times."; \
                        rm -f press.mem; \
	else \
			echo "\033[0;31m============================== TEST FAILED ==============================\033[0m"; \
                        gtkwave --save simonsays.gtkw; \
	fi

$(BUILD)/$(PROJ).json : $(ICE) $(SRC) $(PINMAP) Makefile
	verilator -Wno-PINMISSING --lint-only -Werror-latch $(SRC) $(PROJ).sv $(submodules) --top-module top -Wno-CASEINCOMPLETE
	~ece270/bin/generate-press-mem > press.mem
	# if build folder doesn't exist, create it
	mkdir -p $(BUILD)
	# synthesize using Yosys
	$(YOSYS) -p "read_verilog -sv -noblackbox $(FILES); synth_ice40 -top ice40hx8k -json $(BUILD)/$(PROJ).json"

$(BUILD)/$(PROJ).asc : $(BUILD)/$(PROJ).json
	# Place and route using nextpnr
	$(NEXTPNR) --hx8k --package ct256 --pcf $(PINMAP) --asc $(BUILD)/$(PROJ).asc --json $(BUILD)/$(PROJ).json 2> >(sed -e 's/^.* 0 errors$$//' -e '/^Info:/d' -e '/^[ ]*$$/d' 1>&2)

$(BUILD)/$(PROJ).bin : $(BUILD)/$(PROJ).asc
	# Convert to bitstream using IcePack
	icepack $(BUILD)/$(PROJ).asc $(BUILD)/$(PROJ).bin

flash: $(BUILD)/$(PROJ).bin
	iceprog $(BUILD)/$(PROJ).bin

cram: $(BUILD)/$(PROJ).bin
	iceprog -S $(BUILD)/$(PROJ).bin

time: $(BUILD)/$(PROJ).asc
	icetime -p $(PINMAP) -P $(FOOTPRINT) -d $(TIMEDEV) $<

clean:
	rm -rf build/ simonsays_dir/
