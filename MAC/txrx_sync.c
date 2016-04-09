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

#include "txrx_sync.h"

TimingInfo TXTime;
TimingInfo RXTime;

int __ext_setRXTime(TimingInfo time)
{
	memcpy(&RXTime, &time, sizeof(TimingInfo));
	return 0;
}

TimingInfo __ext_getRXTime()
{
	return RXTime;
}

int __ext_setTXTime(TimingInfo time)
{
	memcpy(&TXTime, &time, sizeof(TimingInfo));
	return 0;
}

TimingInfo __ext_getTXTime()
{
	return TXTime;
}


/*
int __ext_setRXTime(int32 counter, int16 frame, int16 subFrame, int16 slot, int16 symbol, int16 sample)
{
	RXTime.sampleCounter = counter;
	RXTime.frame = frame;
	RXTime.subFrame = subFrame;
	RXTime.slot = slot;
	RXTime.symbol = symbol;
	RXTime.sample = sample;
	return 0;
}

int32 __ext_getRXCounter()
{
	return RXTime.sampleCounter;
}

int16 __ext_getRXFrame()
{
	return RXTime.frame;
}

int16 __ext_getRXSubFrame()
{
	return RXTime.subFrame;
}

int16 __ext_getRXSlot()
{
	return RXTime.slot;
}

int16 __ext_getRXSymbol()
{
	return RXTime.symbol;
}

int16 __ext_getRXSample()
{
	return RXTime.sample;
}
*/

int8 rapidID = -1;

int __ext_setRAPID(int8 rapid)
{
	rapidID = rapid;
	return 0;
}

int8 __ext_getRAPID()
{
	return rapidID;
}


int32 __ext_getEnergyDetectionThreshold() {
	return energyDetectionThreshold;
}

