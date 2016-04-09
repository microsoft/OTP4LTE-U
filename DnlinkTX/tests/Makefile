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

PP   = ../../Library/C/CompilerVS/preprocesscompile-$(COMPILER).sh
DIFF = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff 

# Do not delete outfiles to allow quick recheck of failed tests
# use make clean to get rid of them
.PRECIOUS: %.outfile binin%.outfile

# Targets
TGTS = $(patsubst %.zir, %.test, $(wildcard *.zir))

all: $(TGTS)

%.out : %.zir
# Sometimes inlining and loop unrolling creates too much code and slows down C compilation
# so we want to avoid that
#	EXTRAOPTS="$(EXTRAOPTS) --bounds-check --no-fold --no-exp-fold" $(PP) $< $@
	DEFINES="-DTEST=1 $(DEFINES)" EXTRAOPTS="$(EXTRAOPTS) --bounds-check" $(PP) $< $@



binin%.outfile: binin%.out binin%.infile
	./$< --input=file \
             --input-file-name=binin$*.infile \
             --input-file-mode=bin \
             --output-file-name=$@ \
             --output-file-mode=dbg


%.outfile: %.out %.infile
	./$< --input=file \
             --input-file-name=$*.infile \
             --input-file-mode=dbg \
             --output-file-name=$@ \
             --output-file-mode=dbg



%.test: %.outfile
# BOZIDAR: Due to imprecissions in fixed point, 0.7 is enough
#	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.9
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.8 -t 3000 -c 


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
