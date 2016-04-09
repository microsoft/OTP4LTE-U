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
#include "types.h"

// LTE timing structure
#ifndef STRUCT_TIMINGINFO
#define STRUCT_TIMINGINFO
typedef struct {
	int16 frame;
	int16 subFrame;
	int16 slot;
	int16 symbol;
	int16 sample;
} TimingInfo;
#endif


const int32 LTE_SlotLen = (2048 + 160) + 6 * (2048 + 144);
const int32 LTE_SubFrameLen = LTE_SlotLen * 2;
const int32 LTE_FrameLen = LTE_SlotLen * 2 * 10;


int __ext_setRXTime(TimingInfo time);
TimingInfo __ext_getRXTime();
int __ext_setTXTime(TimingInfo time);
TimingInfo __ext_getTXTime();

int __ext_setRAPID(int8 rapid);
int8 __ext_getRAPID();

extern int32 energyDetectionThreshold;
int32 __ext_getEnergyDetectionThreshold();

