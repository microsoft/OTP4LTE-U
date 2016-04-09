#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "..\..\mac_tx_out_queue.h"


int test_mac_tx_out_queue()
{
	MACDLPacket pkt1, pkt2, pkt3, pkt4, pkt5, pkt6, pkt7, pkt8;
	int ptr, ptr1;

	pkt1.frame = 0;
	pkt1.subframe = 0;
	pkt1.packetType = 1;

	pkt2.frame = 8;
	pkt2.subframe = 5;
	pkt2.packetType = 2;

	pkt3.frame = 4;
	pkt3.subframe = 3;
	pkt3.packetType = 3;

	pkt4.frame = 4;
	pkt4.subframe = 2;
	pkt4.packetType = 4;

	pkt5.frame = 4;
	pkt5.subframe = 2;
	pkt5.packetType = 5;

	pkt6.frame = 4;
	pkt6.subframe = 3;
	pkt6.packetType = 6;

	pkt7.frame = 8;
	pkt7.subframe = 5;
	pkt7.packetType = 7;

	pkt8.frame = 0;
	pkt8.subframe = 0;
	pkt8.packetType = 8;

	init_mac_tx_out_queue();

	add_mac_tx_out_queue(pkt1);
	ptr = find_mac_tx_out_queue(0, 0);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 0);
	assert(outQueue[ptr].pkt.subframe == 0);
	assert(outQueue[ptr].pkt.packetType == 1);

	add_mac_tx_out_queue(pkt8);
	ptr1 = find_mac_tx_out_queue(0, 0);
	ptr = get_next_mac_tx_out_queue(ptr1);
	assert(ptr != -1);
	assert(ptr1 != -1);
	assert(outQueue[ptr].pkt.frame == 0);
	assert(outQueue[ptr].pkt.subframe == 0);
	assert(outQueue[ptr].pkt.packetType == 8);
	
	delete_mac_tx_out_queue(outQueue[ptr1].pkt.frame, outQueue[ptr1].pkt.subframe);
	assert(outQueueSize == 0);

	add_mac_tx_out_queue(pkt1);
	add_mac_tx_out_queue(pkt2);
	add_mac_tx_out_queue(pkt3);
	add_mac_tx_out_queue(pkt4);
	add_mac_tx_out_queue(pkt5);
	add_mac_tx_out_queue(pkt6);
	add_mac_tx_out_queue(pkt7);
	add_mac_tx_out_queue(pkt8);

	ptr = find_mac_tx_out_queue(0, 0);
	assert(ptr != -1);
	ptr = get_next_mac_tx_out_queue(ptr);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 0);
	assert(outQueue[ptr].pkt.subframe == 0);
	assert(outQueue[ptr].pkt.packetType == 8);

	delete_mac_tx_out_queue(0, 0);

	ptr = find_mac_tx_out_queue(4, 3);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 4);
	assert(outQueue[ptr].pkt.subframe == 3);
	assert(outQueue[ptr].pkt.packetType == 3);

	ptr = get_next_mac_tx_out_queue(ptr);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 4);
	assert(outQueue[ptr].pkt.subframe == 3);
	assert(outQueue[ptr].pkt.packetType == 6);

	delete_mac_tx_out_queue(4, 3);

	ptr = find_mac_tx_out_queue(3, 1);
	assert(ptr == -1);

	ptr = find_mac_tx_out_queue(4, 2);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 4);
	assert(outQueue[ptr].pkt.subframe == 2);
	assert(outQueue[ptr].pkt.packetType == 4);

	ptr = get_next_mac_tx_out_queue(ptr);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 4);
	assert(outQueue[ptr].pkt.subframe == 2);
	assert(outQueue[ptr].pkt.packetType == 5);

	delete_mac_tx_out_queue(4, 2);
	delete_mac_tx_out_queue(6, 5);

	ptr = find_mac_tx_out_queue(7, 0);
	assert(ptr == -1);

	ptr = find_mac_tx_out_queue(8, 5);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 8);
	assert(outQueue[ptr].pkt.subframe == 5);
	assert(outQueue[ptr].pkt.packetType == 2);

	ptr = get_next_mac_tx_out_queue(ptr);
	assert(ptr != -1);
	assert(outQueue[ptr].pkt.frame == 8);
	assert(outQueue[ptr].pkt.subframe == 5);
	assert(outQueue[ptr].pkt.packetType == 7);

	delete_mac_tx_out_queue(8, 5);
	assert(outQueueSize == 0);

	return 1;
}