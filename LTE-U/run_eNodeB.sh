#!/bin/bash

# 1 slot is 7 OFDM symbols: (2048 + 160) + (2048 + 144) * 6 = 15360
# --TX-sora-tx-buffer-size=32768 corresponds roughly to 2 slots = subframe
#


################################
### MAC 
################################
# We choose --TX-sora-tx-buffer-size=8192 and cmd_fifo_queue_size=8 to get enough buffering
# and 2.1ms queueing overhead (8192 * 8 / 30.72MHz)


#comment out by Lei
#./mac.out \
#    --TX-sora-sample-rate=15360000 --RX-sora-sample-rate=15360000 \
#    --TX-sora-central-frequency=740000000 --RX-sora-central-frequency=710000000 --LTE-band=17 \
#    --TX-input=ip --TX-output=sora --TX-sora-gain=0 --TX-sora-radio-id=0 \
#    --RX-input=sora --RX-output=memory --RX-sora-radio-id=1 \
#    --TX-heap-size=40000000  --RX-heap-size=40000000 \
#    --TX-sora-tx-buffer-size=32768 \
#    --energy-detection-threshold=200000000 --debug-level=0 --test-IP=on \
#    --Downlink-NRB=-1 --Downlink-MCS=8 --Empty-SubF=1,3,7,9 --Data-SubF=4,8

#comment out by Lei

ONCYCLE=$1
OFFCYCLE=$2
TXGAIN=$3

ctsframes=$4

NRB=49
bias=-18

#samplerate=15360000
#bandwidth=15360000


samplerate=30720000
bandwidth=20000000

#freq=$((2456985000+19148))
#freq=$((2457000000))

freqshift=0
freq=$((2462000000-freqshift))

./mac.out \
    --TX-sora-sample-rate=$samplerate --RX-sora-sample-rate=15360000 \
    --TX-sora-bandwidth=$bandwidth \
    --TX-sora-central-frequency=$freq --RX-sora-central-frequency=2462000000  \
    --TX-output=sora --TX-sora-radio-id=0 \
    --RX-input=sora --RX-output=memory --RX-sora-radio-id=1 \
    --TX-heap-size=1000000000  --RX-heap-size=40000000 \
    --TX-sora-tx-buffer-size=32768 \
    --TX-sora-gain=$TXGAIN \
    --energy-detection-threshold=200000000 --debug-level=0 --test-IP=on \
    --TX-bladerf-dcbias=$bias \
    --Downlink-NRB=$NRB \
    --TX-oncycle=$ONCYCLE \
    --TX-offcycle=$OFFCYCLE \
    --TX-cts-subframes=$ctsframes




#--debug-level=16259 - Print RRC + Print IP head+data + RRC_data + L2 PDSCH+PUSCH+PUCCH + L1 PRACH info + CRC failure
#--debug-level=16131 - Print RRC + Print IP head+data + RRC_data + L2 PUSCH+PUCCH + L1 PRACH info + CRC failure
#--debug-level=15389 - Print RRC + Print IP head+data + RRC_data + CRC failure
#--debug-level=7297 - Print RRC + Print IP head + RRC_data + PDSCH + CRC failure
#--debug-level=7169 - Print RRC + Print IP head + RRC_data + CRC failure
#--debug-level=3073 - Print RRC + Print IP head + CRC failure
#--debug-level=3713 - Print RRC + Print IP head + DLSCH + ULSCH + CRC failure
#--debug-level=1795 - Print RRC + L2 PUSCH+PUCCH + L1 PRACH info + CRC failure
#--test-IP=on - use custom debugging IP responder
#--TX-sora-tx-buffer-size=4096 --TX-cmd-fifo-queue-size=4 --TX-no-tx-bufs=4
#--TX-sora-tx-buffer-size=32768 --TX-cmd-fifo-queue-size=16 --TX-no-tx-bufs=16
#--TX-sora-tx-buffer-size=4096 | 8192 | 16384 | 32768 
#--TX-heap-size=1000000000 --RX-heap-size=1000000000

# Note: energy-detection-threshold=0 means no energy detection printout debugging
#--energy-detection-threshold=900000        -- With 20 dB attenuation
# Threshold without attenuation but false positives occur on uplink transmissions
#--energy-detection-threshold=9000000 --debug-level=1 --test-IP=off

# WARNING: The two offsets have to be the same! UE probably uses a single clock 
# and the spectral distance between TX and RX is fixed!
# Note: the following gives more accurate frequency for SORA12 in R&S RHS8 but fails with dongles...
#--TX-sora-freqency-offset=8 --RX-sora-freqency-offset=8 \
# The following gives more accurate frequency for SORA10 in R&S RHS8 but fails with dongles...
#--TX-sora-freqency-offset=4 --RX-sora-freqency-offset=4 \

# Note: gain can be adjusted
# Large gain (used without physical attenuation), causing high SNR: --TX-sora-gain=2560
# Small gain (used with physical attenuation), causing low SNR but still working: --TX-sora-gain=1000

# AT&T Band 17
# --TX-sora-central-frequency=740 --RX-sora-central-frequency=710 --LTE-band=17 \
# Verizon Band 13
# --TX-sora-central-frequency=751 --RX-sora-central-frequency=782 --LTE-band=13 \

# Note:
# --Downlink-MCS=<mcs> sets MCS and chooses the smalles TBS larger than the packet and the corresponding NBR
# --Downlink-NRB=<nrb> sets NRB and chooses the smalles TBS larger than the packet
# if --Downlink-MCS == -1 we check --Downlink-NRB and if --Downlink-NRB == -1 we use hardcoded values (TBS=153*8, NRB=12)






################################
### MAC for waveform dump
################################
#./mac.out --TX-input=dummy --TX-dummy-samples=12800 --TX-output=sora --TX-sora-gain=4096 --TX-sora-radio-id=0 --TX-sora-central-frequency=739 --TX-sora-freqency-offset=0 --RX-input=sora --RX-sora-radio-id=1 --RX-sora-central-frequency=709  --RX-output=file --TX-heap-size=40000000 --RX-heap-size=40000000 --RX-output-file-name=out.bin --RX-output-file-mode=bin --RX-output-buffer-size=64000000 --TX-sora-tx-buffer-size=32768 --TX-cmd-fifo-queue-size=16 --TX-no-tx-bufs=16


#./mac.out --TX-input=dummy --TX-dummy-samples=12800 --TX-output=sora --TX-sora-gain=4096 --TX-sora-radio-id=0 --TX-sora-central-frequency=739 --RX-input=sora --RX-output=file --RX-output-file-name=out.bin --RX-output-file-mode=bin --RX-sora-radio-id=1 --RX-sora-central-frequency=739 --TX-sora-tx-buffer-size=32768


#./mac.out --TX-input=file --TX-input-file-name=genFrames0_1023.dat --TX-input-file-mode=bin --TX-output=sora --TX-sora-gain=4096 --TX-sora-radio-id=0 --TX-sora-central-frequency=739 --RX-input=sora --RX-output=dummy --RX-sora-radio-id=1 --RX-sora-central-frequency=739 --TX-sora-tx-buffer-size=32768 --TX-heap-size=5000000000 --TX-input-file-repeat=*


# Send the preprepared file
#./mac.out --TX-input=file --TX-input-file-mode=bin --TX-input-file-repeat=* --TX-input-file-name=C:\\Users\\bozidar\\Documents\\Code\\VSTFMSRC\\ExperimentalSoraCode\\TXRXwNewFirmware\\target\\i386\\genFrames0_1023.dat --TX-output=sora --TX-sora-gain=4096 --TX-sora-radio-id=0 --TX-sora-central-frequency=739 --RX-sora-freqency-offset=0 --RX-input=sora --RX-output=dummy --RX-sora-radio-id=1 --RX-sora-central-frequency=709 --TX-sora-tx-buffer-size=32768
