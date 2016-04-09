# ZIRIA_ROOT has to be set as an environment variable
ZIRIA_ROOT_CYGW=$(shell cygpath "$(ZIRIA_ROOT)")


# Disable default rules for C files
.SUFFIXES:

COMPILER?=vs

PP = ../../scripts/preprocesscompile-mac-$(COMPILER).sh
CP = ../../scripts/ccompile-mac-$(COMPILER).sh
DIFF  = $(ZIRIA_ROOT_CYGW)/tools/BlinkDiff

# Do not delete outfiles to allow quick recheck of failed tests
# use make clean to get rid of them
.PRECIOUS: %.outfile

# Targets
vpath %.c ../../csrc/mac
TGTS = $(patsubst %.zir, %.test, $(wildcard *.zir))

all: $(TGTS)



%.c : %.zir
	EXTRAOPTS='$(EXTRAOPTS) --name=_$(basename $@)' $(PP) $< $(basename $@)

mac.out: rx.c tx.c
	EXTRAOPTS='$(EXTRAOPTS)' $(CP) mac.out

mac.outfile: mac.out
	./mac.out --TX-input=dummy --TX-dummy-samples=12800 --TX-output=file \
	--TX-output-file-mode=dbg --TX-output-file-name=$@ --RX-input=dummy --RX-output=dummy

%.test: %.outfile
	$(DIFF) $(DIFFOPTS) -f $< -g $<.ground -d -v -n 0.9



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
