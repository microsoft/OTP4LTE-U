
all: test-LTE-all test-LTE-all-clean test-LTE-opt-all test-LTE-MAC

clean: test-LTE-all-clean


# LTE tests
################################################################

test-LTE-all: test-LTE-DnlinkTX test-LTE-UplinkRX test-LTE-eNodeB test-LTE-MAC
test-LTE-opt-all: test-LTE-DnlinkTX-opt test-LTE-UplinkRX-opt test-LTE-eNodeB-opt test-LTE-MAC
test-LTE-pedantic-all: test-LTE-DnlinkTX-pedantic test-LTE-UplinkRX-pedantic test-LTE-eNodeB-pedantic test-LTE-MAC
test-LTE-all-clean: test-LTE-DnlinkTX-clean test-LTE-UplinkRX-clean test-LTE-eNodeB-clean test-LTE-MAC-clean

test-LTE-perf-all: test-LTE-DnlinkTX-perf test-LTE-UplinkRX-perf test-LTE-eNodeB-perf
test-LTE-perf-all-clean: test-LTE-DnlinkTX-perf-clean test-LTE-UplinkRX-perf-clean test-LTE-eNodeB-perf-clean



# Dnlink LTE tests

test-LTE-DnlinkTX:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE DnlinkTX tests"
	@echo ""
	make -C DnlinkTX/tests && make -C DnlinkTX/tests clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE DnlinkTX tests"
	@echo ""
	@echo ""

test-LTE-DnlinkTX-clean:
	make -C DnlinkTX/tests clean

test-LTE-DnlinkTX-opt: 
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE DnlinkTX optimized tests"
	@echo ""
	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C DnlinkTX/tests && make -C DnlinkTX/tests clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE DnlinkTX optimized tests"
	@echo ""
	@echo ""

test-LTE-DnlinkTX-pedantic: 
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE DnlinkTX pedantic tests"
	@echo ""
	make -C DnlinkTX/tests && make -C DnlinkTX/tests clean
	EXTRAOPTS='--no-fold --no-exp-fold' make -C DnlinkTX/tests && make -C DnlinkTX/tests clean
	EXTRAOPTS='--autolut' make -C DnlinkTX/tests && make -C DnlinkTX/tests clean
	DIFFOPTS='-p' EXTRAOPTS='--vectorize' make -C DnlinkTX/tests && make -C DnlinkTX/tests clean
	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C DnlinkTX/tests && make -C DnlinkTX/tests clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE DnlinkTX pedantic tests"
	@echo ""
	@echo ""

test-LTE-DnlinkTX-perf:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE DnlinkTX perf tests"
	@echo ""
	make -C DnlinkTX/perf
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE DnlinkTX perf tests"
	@echo ""
	@echo ""

test-LTE-DnlinkTX-perf-clean:
	make -C DnlinkTX/perf clean



# Uplink LTE tests

test-LTE-UplinkRX:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE UplinkRX tests"
	@echo ""
	make -C UplinkRX/tests && make -C UplinkRX/tests clean
# Temporarily disabled until the test is fixed
#	make -C UplinkRX/PRACH/tests && make -C UplinkRX/PRACH/tests clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE UplinkRX tests"
	@echo ""
	@echo ""

test-LTE-UplinkRX-clean:
	make -C UplinkRX/tests clean

test-LTE-UplinkRX-opt: 
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE UplinkRX optimized tests"
	@echo ""
	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C UplinkRX/tests && make -C UplinkRX/tests clean
# Temporarily disabled until the test is fixed
#	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C UplinkRX/PRACH/tests && make -C UplinkRX/PRACH/tests clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE UplinkRX optimized tests"
	@echo ""
	@echo ""

test-LTE-UplinkRX-pedantic:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE UplinkRX pedantic tests"
	@echo ""
	make -C UplinkRX/tests && make -C UplinkRX/tests clean
	EXTRAOPTS='--no-fold --no-exp-fold' make -C UplinkRX/tests && make -C UplinkRX/tests clean
	EXTRAOPTS='--autolut' make -C UplinkRX/tests && make -C UplinkRX/tests clean
	DIFFOPTS='-p' EXTRAOPTS='--vectorize' make -C UplinkRX/tests && make -C UplinkRX/tests clean
	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C UplinkRX/tests && make -C UplinkRX/tests clean
# Temporarily disabled until the test is fixed
#	make -C UplinkRX/PRACH/tests && make -C UplinkRX/PRACH/tests clean
#	EXTRAOPTS='--no-fold --no-exp-fold' make -C UplinkRX/PRACH/tests && make -C UplinkRX/PRACH/tests clean
#	EXTRAOPTS='--autolut' make -C UplinkRX/PRACH/tests && make -C UplinkRX/PRACH/tests clean
#	DIFFOPTS='-p' EXTRAOPTS='--vectorize' make -C UplinkRX/PRACH/tests && make -C UplinkRX/PRACH/tests clean
#	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C UplinkRX/PRACH/tests && make -C UplinkRX/PRACH/tests clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE UplinkRX pedantic tests"
	@echo ""
	@echo ""

test-LTE-UplinkRX-perf:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE UplinkRX perf tests"
	@echo ""
	make -C UplinkRX/perf
	make -C UplinkRX/PRACH/perf
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE UplinkRX perf tests"
	@echo ""
	@echo ""

test-LTE-UplinkRX-perf-clean:
	make -C UplinkRX/perf clean
	make -C UplinkRX/PRACH/perf clean



# eNodeB (full) LTE tests

test-LTE-eNodeB:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE eNodeB tests"
	@echo ""
	make -C eNodeB test && make -C eNodeB clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE eNodeB tests"
	@echo ""
	@echo ""

test-LTE-eNodeB-clean:
	make -C eNodeB clean

test-LTE-eNodeB-opt: 
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE eNodeB optimized tests"
	@echo ""
	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C eNodeB test && make -C eNodeB clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE eNodeB optimized tests"
	@echo ""
	@echo ""

test-LTE-eNodeB-pedantic:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE eNodeB pedantic tests"
	@echo ""
	make -C eNodeB test && make -C eNodeB clean
	EXTRAOPTS='--no-fold --no-exp-fold' make -C eNodeB test && make -C eNodeB clean
	EXTRAOPTS='--autolut' make -C eNodeB test && make -C eNodeB clean
	DIFFOPTS='-p' EXTRAOPTS='--vectorize' make -C eNodeB test && make -C eNodeB clean
	DIFFOPTS='-p' EXTRAOPTS='--vectorize --autolut' make -C eNodeB test && make -C eNodeB clean
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE eNodeB pedantic tests"
	@echo ""
	@echo ""

test-LTE-eNodeB-perf:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE eNodeB tests"
	@echo ""
	make -C eNodeB perf
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE eNodeB tests"
	@echo ""
	@echo ""

test-LTE-eNodeB-perf-clean:
	make -C eNodeB clean



# MAC LTE tests

test-LTE-MAC:
	@echo ""
	@echo ""
	@echo ">>>>>>>>>>>>>>> LTE MAC tests"
	@echo ""
	make -C MAC/CompilerVS
	@echo ""
	@echo "<<<<<<<<<<<<<<< LTE MAC tests"
	@echo ""
	@echo ""

test-LTE-MAC-clean:
	make -C MAC/CompilerVS clean
