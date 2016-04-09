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

PP   = ../../Library/C/CompilerVS/preprocesscompile-$(COMPILER)-inline.sh
DIFF = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff 

# Do not delete outfiles to allow perf rerun without recompilation
.PRECIOUS: %.out

# Targets
TGTS = $(patsubst %.zir, %.perf, $(wildcard *.zir))

all: $(TGTS)

%.out : %.zir
# Sometimes inlining and loop unrolling creates too much code and slows down C compilation
# so we want to avoid that
#	EXTRAOPTS="$(EXTRAOPTS) --bounds-check --no-fold --no-exp-fold" $(PP) $< $@
	DEFINES="-DFAKE_TIME=1 -DPERF=1 $(DEFINES)" EXTRAOPTS='--vectorize --autolut $(EXTRAOPTS)' $(PP) $< $@
#	DEFINES="-DPERF=1 $(DEFINES)" EXTRAOPTS='--vectorize  $(EXTRAOPTS)' $(PP) $< $@

%.perf: %.out 
	./$< --input=dummy --dummy-samples=100000000 --output=dummy

clean:
	rm -f ./*expanded
	rm -f ./*exe ./*exec
	rm -f ./*.outfile
	rm -f ./*.out
	rm -f ./compout ./perfout
	rm -f ./*.c

all-clean: clean
	rm -f ./*~
