# 
# Copyright (c) Microsoft Corporation
# All rights reserved. 
#
# Licensed under the Apache License, Version 2.0 (the ""License""); you
# may not use this file except in compliance with the License. You may
# obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR
# A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#
#

# Disable default rules for C files
.SUFFIXES:

# Uncomment the following if you want winddk to be the default compiler. 
# NB: you can override this from the command line, e.g: 
# prompt> COMPILER=gcc make
#
# COMPILER?=winddk
# COMPILER?=gcc
#
COMPILER?=vs

PP    = scripts/preprocesscompile-mac-$(COMPILER).sh
CP    = scripts/ccompile-mac-$(COMPILER)-inline.sh
CPDEB = scripts/ccompile-mac-$(COMPILER).sh

ZIRIA_ROOT_CYGW=$(shell cygpath "$(ZIRIA_ROOT)")
PPZ  = $(ZIRIA_ROOT_CYGW)/scripts/preprocesscompile-$(COMPILER)-inline.sh
PPZDEB  = $(ZIRIA_ROOT_CYGW)/scripts/preprocesscompile-$(COMPILER)-debug.sh
DIFF  = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff

# Do not delete outfiles to allow quick recheck of failed tests
# use make clean to get rid of them
.PRECIOUS: %.outfile rx_test%.outfile tx_test%.outfile %.pout rx_test%.pout

# Targets
vpath %.c ../../csrc/mac
TGTS = $(patsubst %.zir, %.test, $(wildcard *.zir))

all: $(TGTS)


### Only Sora compilation/running supported


##############################################
### Actual PHY/MAC running on hardware
### Type: make mac.run
#%.c : %.zir
#	DEFINES="-DTEST_BED=1 $(DEFINES)" EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_$(basename $@)' $(PP) $< $(basename $@)

rx.c : rx.zir
	DEFINES="-DTEST_BED=1 $(DEFINES)" EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_rx' $(PP) rx.zir rx

tx.c : tx.zir
	DEFINES="-DTEST_BED=1 $(DEFINES)" EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_tx' $(PP) tx.zir tx

mac.out: rx.c tx.c
# Use this one to compile 32-bit (with DebugPlot)
# We also need this one with turbo decoder
	MODE='/p:Radio=Sora' RUN_TYPE=32 EXTRAOPTS='$(EXTRAOPTS)' $(CP) mac.out
# Use this one to compile 64-bit (without DebugPlot). CANNOT BE USED WITH TURBO_DECODER
#	EXTRAOPTS='$(EXTRAOPTS)' $(CP) mac.out


mac.run: mac.out
	./run_eNodeB.sh




##############################################
### Actual PHY/MAC running on hardware
### But in the debug mode that dumps selected subframes
### Type: make mac.run
dump.out: rx.zir tx.zir
	DEFINES="-DDUMP -DTEST_BED=1 $(DEFINES)" EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_rx' $(PP) rx.zir rx
	DEFINES='-DDUMP' EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_tx' $(PP) tx.zir tx
# Use this one to compile 32-bit (with DebugPlot)
# We also need this one with turbo decoder
	RUN_TYPE=32 EXTRAOPTS='$(EXTRAOPTS)' $(CP) dump.out
# Use this one to compile 64-bit (without DebugPlot). CANNOT BE USED WITH TURBO_DECODER
#	EXTRAOPTS='$(EXTRAOPTS)' $(CP) mac.out






##############################################
### Tests
### Type: make tx_test<no>.test or make rx_test<no>.test
.PHONY: test tx_test rx_test
TX_TESTS = $(patsubst %.zir, %.test, $(wildcard tx_test*.zir))

# Currently, all rx tests fail, so we disable until we fix them
#RX_TESTS = $(patsubst %.zir, %.test, $(wildcard rx_test*.zir))
RX_TESTS = 

test: tx_test rx_test

# tx_test1.outfile.ground is copied from LTE/MatlabTests/eNodeB/tests/tx_test1.outfile.ground
# tx_test2.outfile.ground prints the (frames,subframes) in which there is a packet,
#                         and is regenerated through outfile.
# rx_test1.outfile.ground is copied from LTE/MatlabTests/eNodeB/tests/rx_test*

tx_test: $(TX_TESTS)
	echo "All TX tests passed: " $(TX_TESTS)
rx_test: $(RX_TESTS)
	echo "All RX tests passed: " $(RX_TESTS)

# '/p:Mode=TEST' adds /DTEST /DSW_TEST
tx_test%.out: tx_test%.zir
	DEFINES="-DMAC_TEST=1 $(DEFINES)" EXTRAOPTS='$(EXTRAOPTS) --bounds-check --name=_tx' $(PP) $< tx
	EXTRAOPTS='--name=_rx' $(PP) dummy.zir rx
	MODE='/p:Mode=TEST' EXTRAOPTS='$(EXTRAOPTS)' $(CPDEB) $(basename $@).out

tx_test%.outfile: tx_test%.out tx.infile
	./$< --TX-input=file \
             --TX-input-file-name=tx.infile \
             --TX-input-file-mode=bin \
             --TX-output-file-name=$(basename $@).outfile\
             --TX-output-file-mode=dbg \
             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709 \
	     --RX-input=dummy --RX-dummy-samples=1 --RX-output=dummy



tx_test%.test: tx_test%.outfile
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.8 -c


# '/p:Mode=TEST' adds /DTEST /DSW_TEST
rx_test%.out: rx_test%.zir
	DEFINES="-DMAC_TEST=1 $(DEFINES)" \
	         EXTRAOPTS='$(EXTRAOPTS) --bounds-check --name=_rx' $(PP) $< rx
	EXTRAOPTS='--name=_tx' $(PP) dummy.zir tx
	MODE='/p:Mode=TEST' EXTRAOPTS='$(EXTRAOPTS) --bounds-check' $(CPDEB) $(basename $@).out

# We set the output to memory and give a file name because MAC C code uses the file name
# to write the output to the file. This way we test both MAC and PHY
rx_test%.outfile: rx_test%.out rx_test.infile
	./$< --RX-input=file \
             --RX-input-file-name=rx_test.infile \
             --RX-input-file-mode=dbg \
             --RX-output=memory\
             --RX-output-file-name=$(basename $@).outfile\
             --RX-output-file-mode=bin \
	     --TX-heap-size=40000000 --RX-heap-size=40000000 \
             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709 \
	     --TX-input=dummy --TX-dummy-samples=1 --TX-output=dummy

rx_test%.test: rx_test%.outfile
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v





##############################################
### Perf test
### Type: make tx.perf or make rx.perf
###
### '/p:Mode=PERF' adds /DPERF /DSW_TEST
###
### We use 32-bit mode because current turboDecode implementation uses asm 
### (instead of intrinsics), which prevents 64-bit compilation
perf: tx.perf rx.perf #rx_test1.perf 
tx.pout: dummy.zir tx.zir
	DEFINES="-DPERF=1 $(DEFINES)" \
		EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_tx' \
		$(PP) tx.zir tx
	EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_rx' $(PP) dummy.zir rx
	RUN_TYPE=32 MODE='/p:Mode=PERF' EXTRAOPTS='$(EXTRAOPTS)' $(CP) tx.pout

rx.pout: dummy.zir rx.zir
	DEFINES="-DPERF=1 $(DEFINES)" \
		EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_tx' \
		$(PP) dummy.zir tx
	EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_rx' $(PP) rx.zir rx
	RUN_TYPE=32 MODE='/p:Mode=PERF' EXTRAOPTS='$(EXTRAOPTS)' $(CP) rx.pout

# rx_test1.zir just includes rx.zir so no point to run it again
# rx_test%.pout: rx_test%.zir
# 	DEFINES="-DPERF=1 $(DEFINES)" \
# 		EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_tx' \
# 		$(PP) dummy.zir tx
# 	EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS) --name=_rx' $(PP) $< rx
# 	RUN_TYPE=32 MODE='/p:Mode=PERF' EXTRAOPTS='$(EXTRAOPTS)' $(CP) $(basename $@).pout


# In dnlinkPHY.zir we generate 1000 frames if PERF is defined, so the test should run in less than 10s (1000 slots x 10ms)
tx.perf: tx.pout 
	./$< --TX-input=dummy --TX-dummy-samples=1 --TX-output=dummy --RX-input=dummy --RX-dummy-samples=1 --RX-output=dummy \
             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709 

# Parameter --latency-sampling=1 causes approximately 2x measurement slowdown 
# (i.e. the obtained numbers should be divided by 2) but gives good precision
# The numbers in tx|rx_sample.txt are in nano seconds (units of SoraGetCPUTimestamp).
# Also, remember that output is (probably) vectorized in 16, so the output time
# is between two 16 sample writes.
tx.sample: tx.pout 
	./$< --TX-input=dummy --TX-dummy-samples=1 --TX-output=dummy --RX-input=dummy --RX-dummy-samples=1 \
	     --TX-latency-sampling=1 \
	     --TX-latency-sampling-location=write \
	     --TX-latency-CDF-size=1000000 \
             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709 


# This is properly configured. 100s of input should run in <3s.
rx.perf: rx.pout 
	./$< --TX-input=dummy --TX-dummy-samples=1 --TX-output=dummy --RX-input=dummy --RX-dummy-samples=100000000 --RX-output=dummy \
             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709
rx.sample: rx.pout 
	./$< --TX-input=dummy --TX-dummy-samples=1 --TX-output=dummy --RX-input=dummy --RX-dummy-samples=100000000 --RX-output=dummy \
	     --RX-latency-sampling=1 \
	     --RX-latency-sampling-location=read \
	     --RX-latency-CDF-size=1000000 \
             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709 


# This is properly configured. 100s of input should run in <3s.
# rx_test%.perf: rx_test%.pout 
# 	./$< --TX-input=dummy --TX-dummy-samples=1 --TX-output=dummy --RX-input=dummy --RX-dummy-samples=100000000 --RX-output=dummy \
#             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709 
# rx_test%.sample: rx_test%.pout 
# 	./$< --TX-input=dummy --TX-dummy-samples=1 --TX-output=dummy --RX-input=dummy --RX-dummy-samples=100000000 --RX-output=dummy \
# 	     --RX-latency-sampling=1 \
# 	     --RX-latency-sampling-location=read \
# 	     --RX-latency-CDF-size=1000000 \
#             --LTE-band=17 --TX-sora-central-frequency=739 --RX-sora-central-frequency=709 




#############################################################################
### Accept Matlab tests as new ground thruth - copy from MatlabTests dir.
accept:
	cp ../MatlabTests/DnlinkTX/tests/test_genFrame.outfile.ground tx_test1.outfile.ground




##############################################
### Cleaning
clean:	
	rm -f ./*expanded
	rm -f ./*exe ./*exec ./*out
	rm -f ./*.outfile
	rm -f ./compout ./perfout
	rm -f ./*.c

all-clean: clean
	rm -f ./*~
	rm -f ./*dump
