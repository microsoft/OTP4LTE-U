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

# ZIRIA_ROOT has to be set as an environment variable
ZIRIA_ROOT_CYGW=$(shell cygpath "$(ZIRIA_ROOT)")

#PP     = ../../Library/C/CompilerVS/preprocesscompile-$(COMPILER)-inline.sh
#PPDEB = ../../Library/C/CompilerVS/preprocesscompile-$(COMPILER).sh

PP    = $(ZIRIA_ROOT_CYGW)/scripts/preprocesscompile-$(COMPILER)-inline.sh
PPDEB = $(ZIRIA_ROOT_CYGW)/scripts/preprocesscompile-$(COMPILER)-debug.sh

DIFF   = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff 

# Do not delete outfiles to allow quick recheck of failed tests
# use make clean to get rid of them
.PRECIOUS: %.outfile binin%.outfile %.out

# Targets
TGTS = $(patsubst %.zir, %.test, $(wildcard *.zir))

all: $(TGTS)

%.out : %.zir
#	C_DEFINES='BLADE_RF' LIBS='bladeRF.lib' EXTRAOPTS='--bounds-check $(E$XTRAOPTS)' $(PPDEB) $< $@
	C_DEFINES='BLADE_RF' LIBS='bladeRF.lib' EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS)' $(PP) $< $@


%.perf: %.out 
	PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	./$< --input=dummy --output=dummy

%.outfile: %.out 
	PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	./$< --input=file \
             --input-file-name=$*.infile \
             --input-file-mode=dbg \
             --output-file-name=$@ \
             --output-file-mode=dbg


DnlinkFREQ:= 2412000000
#DnlinkFREQ:=740000000
TXGAIN:=25

dnlinkPHY.run: dnlinkPHY.out
	PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	     ./$< --input=dummy --output=sdr --heap-size=100000000 --sdr-central-frequency=$(DnlinkFREQ) --sdr-tx-gain=$(TXGAIN) --sdr-sample-rate=15360000



#==================================================================================

dnlink_empty.run: dnlink_empty.out 
	        PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	     ./$< --input=file \
             --input-file-name=dnlink_empty.infile \
              --input-file-mode=dbg --input-file-repeat=*\
             --output=sdr --heap-size=100000000 --sdr-central-frequency=$(DnlinkFREQ) --sdr-tx-gain=$(TXGAIN) --sdr-sample-rate=15360000

#==================================================================================


#740920635 = 740MHz in bladeRF due to shift
#roughly multiply 1.0013


FREQ:=2412000000

test.run: test.out
	PATH="$(PATH)":"/cygdrive/c/Program Files/bladeRF/x64" \
	     ./$< --input=dummy --output=sdr --heap-size=1000000000 \
		  --sdr-central-frequency=$(FREQ) \
		  --sdr-tx-gain=20 \
		  --sdr-sample-rate=15360000


%.test: %.outfile
# BOZIDAR: Due to imprecissions in fixed point, 0.7 is enough
#	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.9
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.8 -c


%.accept : %.outfile
	cp $< $<.ground


clean:
	rm -f ./*expanded
	rm -f ./*exe ./*exec
	rm -f ./*.outfile
	rm -f ./*.out
	rm -f ./compout ./perfout
	rm -f ./*.c

all-clean: clean
	rm -f ./*~
