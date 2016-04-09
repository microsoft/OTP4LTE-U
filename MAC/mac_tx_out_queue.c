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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "mac_tx_out_queue.h"


int outQueueSize;
outQueuePacket outQueue[TX_OUT_QUEUE_SIZE];

void init_mac_tx_out_queue()
{
	memset((void *)outQueue, 0, sizeof(outQueuePacket)* TX_OUT_QUEUE_SIZE);
	outQueueSize = 0;
}




int add_mac_tx_out_queue(MACDLPacket pkt)
{
	int i = 0;
	while (i < outQueueSize && outQueue[i].taken) i++;
	if (i < TX_OUT_QUEUE_SIZE)
	{
		outQueue[i].pkt = pkt;
		outQueue[i].taken = true;
		if (i == outQueueSize) outQueueSize++;
		return 1;
	}
	return 0;
}




// Find the next packet to be transmitted
int find_mac_tx_out_queue(int frame, int subframe)
{
	int ind = -1;

	for (int i = 0; i < outQueueSize; i++)
	{
		if (outQueue[i].taken && outQueue[i].pkt.frame == frame && outQueue[i].pkt.subframe == subframe)
		{
			ind = i;
			break;
		}
	}

	return ind;
}




// "Iterator" over packets found. 
// Returns the next packet with the same (frame, subframe) pair
int get_next_mac_tx_out_queue(int start)
{
	int ind = -1; 
	for (int i = start+1; i < outQueueSize && ind == -1; i++)
	{
		if (outQueue[i].taken)
		{
			if (outQueue[start].pkt.frame == outQueue[i].pkt.frame && outQueue[start].pkt.subframe == outQueue[i].pkt.subframe)
			{
				ind = i;
			}
		}
	}
	return ind;
}




// Remove all the packet with the same (frame,subframe)
void delete_mac_tx_out_queue(int frame, int subframe)
{
	int i;
	for (i = 0; i < outQueueSize; i++)
	{
		if (outQueue[i].taken && outQueue[i].pkt.frame == frame && outQueue[i].pkt.subframe == subframe)
		{
			outQueue[i].taken = false;
		}
	}
	while (outQueueSize > 0 && !outQueue[outQueueSize-1].taken)
	{
		outQueueSize--;
	}
}