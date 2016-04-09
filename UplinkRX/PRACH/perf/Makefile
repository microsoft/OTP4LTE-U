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
	EXTRAOPTS='--vectorize $(EXTRAOPTS)' $(PP) $< $@


%.perf: %.out 
	./$< --input=dummy \
	     --dummy-samples=100000000 \
             --output=dummy


clean:
	rm -f ./*expanded
	rm -f ./*exe ./*exec
	rm -f ./*.outfile
	rm -f ./*.out
	rm -f ./compout ./perfout
	rm -f ./*.c

all-clean: clean
	rm -f ./*~
