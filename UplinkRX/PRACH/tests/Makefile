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

PP   = ../../../Library/C/CompilerVS/preprocesscompile-$(COMPILER).sh
DIFF = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff 

# Do not delete outfiles to allow quick recheck of failed tests
# use make clean to get rid of them
.PRECIOUS: %.out %.outfile

# Targets
TGTS = $(patsubst %.zir, %.test, $(wildcard *.zir))

all: $(TGTS)

%.out : %.zir
	EXTRAOPTS="--bounds-check $(EXTRAOPTS)" $(PP) $< $@


%.outfile: %.out %.infile
	./$< --input=file \
             --input-file-name=$*.infile \
             --input-file-mode=dbg \
             --output-file-name=$@ \
             --output-file-mode=dbg



test_detectUE.outfile: test_detectUE.out test_detectUE.infile
	./$< --input=file \
             --input-file-name=test_detectUE1.infile \
             --input-file-mode=dbg \
             --output-file-name=$@ \
             --output-file-mode=dbg


%.test: %.outfile
# BOZIDAR: Due to imprecissions in fixed point, 0.7 is enough
#	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.9
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.7


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
