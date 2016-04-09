-----------------------------------------
A comment on performance evaluation of TX
-----------------------------------------


Measurements:
-------------

Example numbers (for 10000 subframes = 1000 frames = 10s):
test_addMIBToSubframe: 731973 us
test_addSIB1ToSubframe: 4179345 us
test_addSIB2ToSubframe: 5791229 us
test_addRARToSubframe: 3246241 us
test_modulate: 2032755 us
test_genFrame: 2559021 us



Interpretation:
---------------

- One SIB2 preparation (encoding, interleaving, etc) takes 4179345 us / 10000 ~ 420 us. 
- One OFDM modulation takes 2032755 us / (14*10000) ~ 15 us.
(Sample measurements are roughly 2x-3x that)

test_genFrame adds MIBs once per each frame, adds SIBs (SIB1 once each two frames and SIB2 once each 8 in subframes 0 and 2) and adds RAR (once per frame). So test_genFrame should be:
test_modulate + test_addSIB1ToSubframe/20 + test_addSIB1ToSubframe/50 + test_addRARToSubframe/10 ~ 2.7s, which matches

The challege is that DLSCH preparation happens in between two consecutive chunk writes (and we vectorize in 16), so there is quite a delay when a new DLSCH happens, which may cause occasional buffer overflows. 



Comparison vs eNodeB:
---------------------

eNodeB/tx.perf takes 2882359 us for 1000 frames, compared to 2559021 for genFrame that does the same thing, so no significant performance difference. 


Unit perf tests:
----------------

Tests of individual small blocks
