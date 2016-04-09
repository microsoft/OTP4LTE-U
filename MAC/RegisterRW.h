/*********************************************************************************

MIT License

Copyright (c) 2016 Microsoft

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*********************************************************************************/

#pragma once
#include "sora.h"
#include "sora_radio.h"

typedef enum __DUT_CMD
{
    CMD_INVALID = -1, 
    CMD_RD_REG,
    CMD_WR_REG, 
    CMD_START_RADIO, 
    CMD_STOP_RADIO,
    CMD_TX, 
    CMD_TX_DONE, 
    CMD_INFO, 
    CMD_TRANSFER, 
    CMD_RX_GAIN, 
    CMD_RX_PA,
    CMD_TX_GAIN, 
    CMD_SAMPLE_RATE, 
    CMD_CENTRAL_FREQ,
    CMD_FREQ_OFFSET,
    CMD_DUMP,
    CMD_STOP_TX,
    CMD_DMA,
    CMD_FW_VERSION
}DUT_CMD;


#define DUT_IO_CODE(Cmd) CTL_CODE(\
    FILE_DEVICE_UNKNOWN,\
    0x800 + (Cmd),\
    METHOD_BUFFERED,\
    FILE_ANY_ACCESS)

typedef struct _OFFSET_VALUE
{
    ULONG Offset;
    ULONG Value;
} OFFSET_VALUE, *POFFSET_VALUE;

typedef struct _DEV_RET
{
    HRESULT hResult;
    //UCHAR   data[0];		// This fails during compilation due to warining levels
}DEV_RET, *PDEV_RET;

#define DEVNAME "\\\\.\\HwTest"

extern HANDLE hDeviceHandle;

HANDLE GetDeviceHandle(LPCTSTR szDeviceName);
HRESULT ReadRegister(ULONG Offset, PULONG val);
HRESULT WriteRegister(ULONG Offset, ULONG Value);

HRESULT SetFilterParams(int unit, ulong addr, ulong val);




// Set dnsample = TRUE to downsample the incoming 40MHz signal into the LTE frequency 30.72 MHz
void SetRXDownsampleParameters(bool dnsample);

// Execute to enable TX command queue that allows
// continuous TX without gaps between buffers
// Max queueSize is 64
void SetContinuousTXParameters(ulong queueSize);

// Set upsample = TRUE to upsample the incoming signal on LTE frequency 30.72 MHz into the outgoing 40MHz clock
void SetTXUpsampleParameters(bool upsample);


// Set RXTXSync type
// 0 - RX is entirely from the radio
// 1 - LSB of RX is copied over from TX
// 2 - The entire RX is copied over from TX
void SetTXRXSync(ulong type);
