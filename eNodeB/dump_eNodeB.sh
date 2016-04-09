# 1 slot is 7 OFDM symbols: (2048 + 160) + (2048 + 144) * 6 = 15360
# --TX-sora-tx-buffer-size=32768 corresponds roughly to 2 slots = subframe
#


################################
### MAC for waveform dump
################################
./dump.out --TX-input=dummy --TX-dummy-samples=12800 --TX-output=sora --TX-sora-gain=4096 --TX-sora-radio-id=0 --TX-sora-central-frequency=739 --TX-sora-freqency-offset=0 --RX-input=sora --RX-sora-radio-id=1 --RX-sora-central-frequency=709  --RX-output=file --TX-heap-size=40000000 --RX-heap-size=40000000 --RX-output-file-name=out.bin --RX-output-file-mode=bin --RX-output-buffer-size=64000000 --TX-sora-tx-buffer-size=32768 --TX-cmd-fifo-queue-size=16 --TX-no-tx-bufs=16 --energy-detection-threshold=0
