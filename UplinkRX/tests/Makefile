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
.PRECIOUS: %.outfile binin%.outfile test%_FFT1024.outfile

# Targets
TGTS = $(patsubst %.zir, %.test, $(wildcard *.zir))

all: $(TGTS)

%.out : %.zir
# Sometimes inlining and loop unrolling creates too much code and slows down C compilation
# so we want to avoid that
#	EXTRAOPTS="$(EXTRAOPTS) --bounds-check --no-fold --no-exp-fold" $(PP) $< $@
	DEFINES="-DTEST=1 -DFAKE_TIME=1 $(DEFINES)" EXTRAOPTS="$(EXTRAOPTS) --bounds-check" $(PP) $< $@


# binin%.outfile: binin%.out binin%.infile
binin%.outfile: binin%.out binintest_uplinkPHY_real_samples.infile
	./$< --input=file \
             --input-file-name=binintest_uplinkPHY_real_samples.infile \
             --input-file-mode=bin \
             --output-file-name=$@ \
             --output-file-mode=dbg


%.outfile: %.out %.infile
	./$< --input=file \
             --input-file-name=$*.infile \
             --input-file-mode=dbg \
             --output-file-name=$@ \
             --output-file-mode=dbg


# Test with complex outputs
binintest_c_%.test: binintest_c_%.outfile
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.7 -c -t 100

# Test with binary outputs
binintest_%.test: binintest_%.outfile
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v

# Test with complex outputs
test_c_%.test: test_c_%.outfile
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.7 -c -t 50

# Test with binary outputs
%.test: %.outfile
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v


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
