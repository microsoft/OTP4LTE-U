# Disable default rules for C files
.SUFFIXES:

# Uncomment the following if you want winddk to be the default compiler. 
# NB: you can override this from the command line, e.g: 
# prompt> COMPILER=gcc make
#
# COMPILER?=winddk
# COMPILER?=gcc
# COMPILER?=vs
#
COMPILER?=vs

# ZIRIA_ROOT has to be set as an environment variable
ZIRIA_ROOT_CYGW=$(shell cygpath "$(ZIRIA_ROOT)")

PP   = $(ZIRIA_ROOT_CYGW)/scripts/preprocesscompile-$(COMPILER)-inline.sh
DIFF = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff 

# Do not delete outfiles to allow quick recheck of failed tests
# use make clean to get rid of them
.PRECIOUS: %.outfile %.out

# Targets
TGTS = $(patsubst %.zir, %.perf, $(wildcard *.zir))

all: $(TGTS)

%.out : %.zir
	DEFINES="-DPERF=1 $(DEFINES)" EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS)' $(PP) $< $@

%.perf: %.out 
	./$< --input=dummy \
	     --dummy-samples=100000000 \
             --output=dummy


# Parameter --latency-sampling=1 causes approximately 2x measurement slowdown 
# (i.e. the obtained numbers should be divided by 2) but gives good precision
# Also, remember that output is (probably) vectorized in 16, so the output time
# is between two 16 sample writes.
%.sample: %.out 
	./$< --input=dummy \
	     --dummy-samples=100000000 \
             --output=dummy \
	     --latency-sampling=1 \
	     --latency-sampling-location=write \
	     --latency-CDF-size=100000

clean:
	rm -f ./*expanded
	rm -f ./*exe ./*exec
	rm -f ./*.outfile
	rm -f ./*.out
	rm -f ./compout ./perfout
	rm -f ./*.c

all-clean: clean
	rm -f ./*~
