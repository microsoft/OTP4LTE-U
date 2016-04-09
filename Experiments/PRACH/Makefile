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

ZIRIA_ROOT_CYGW=$(shell cygpath "$(ZIRIA_ROOT)")

PP   = $(ZIRIA_ROOT_CYGW)/scripts/preprocesscompile-$(COMPILER)-inline.sh
DIFF = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff 

# Do not delete outfiles to allow quick recheck of failed tests
# use make clean to get rid of them
.PRECIOUS: %.out

# Targets
TGTS = $(patsubst %.zir, %.perf, $(wildcard *.zir))

all: $(TGTS)

%.out : %.zir
# Removed autolut because it slows down compilation and doesn't help PRACH
#	EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS)' $(PP) $< $@
	C_DEFINES='BLADE_RF' LIBS='bladeRF.lib' EXTRAOPTS='--vectorize $(EXTRAOPTS)' $(PP) $< $@


#%.perf: %.out 
#	./$< --input=dummy \
#	     --dummy-samples=100000000 \
#             --output=dummy

test_detectUE.run: test_detectUE.out 
	PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	./$< --input=sdr --heap-size=1000000000 \
	     --sdr-central-frequency=710000000 \
             --sdr-sample-rate=15360000 \
	     --sdr-rx-gain=20 --input-file-mode=bin --output=dummy

#	     --sdr-rx-gain=-10 --input-file-mode=bin --output=dummy
#	     --sdr-central-frequency=710000000 \


dump.outfile: dump.out 
	PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	./$< --input=file \
             --input-file-name=dump_ipa_2.bin \
             --input-file-mode=bin \
             --output-file-name=$@ \
             --output-file-mode=bin



dump.run: dump.out 
	PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	./$< --input=sdr --heap-size=1000000000 \
	     --sdr-central-frequency=782000000 \
             --sdr-sample-rate=15360000 \
	     --sdr-rx-gain=20 --input-file-mode=bin \
	     --output=file --output-file-mode=bin --output-file-name=dump.bin

#	     --sdr-rx-gain=-10 --input-file-mode=bin \
#	     --sdr-central-frequency=710000000 \

clean:
	rm -f ./*expanded
	rm -f ./*exe ./*exec
	rm -f ./*.outfile
	rm -f ./*.out
	rm -f ./compout ./perfout
	rm -f ./*.c

all-clean: clean
	rm -f ./*~
