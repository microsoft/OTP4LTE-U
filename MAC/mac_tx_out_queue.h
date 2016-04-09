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
#include "mac_structs.h"

// struct and methods for MAC output packet queue


// Ordered list of output TX packets
typedef struct _outQueuePacket {
	MACDLPacket pkt;
	bool taken;
} outQueuePacket;

#define TX_OUT_QUEUE_SIZE	100

extern int outQueueSize;
extern outQueuePacket outQueue[TX_OUT_QUEUE_SIZE];



void init_mac_tx_out_queue();
int add_mac_tx_out_queue(MACDLPacket pkt);
int find_mac_tx_out_queue(int frame, int subframe);
int get_next_mac_tx_out_queue(int start);
void delete_mac_tx_out_queue(int frame, int subframe);

