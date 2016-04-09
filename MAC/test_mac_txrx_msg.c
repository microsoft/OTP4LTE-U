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

#include "mac.h"
#include "mac_tx_out_queue.h"
#include "txrx_sync.h"
#include "mac_msg_parsing.h"



// Dummy variables, to create the same linker environment as in driver.c
BufContextBlock buf_ctx_tx, buf_ctx_rx;
HeapContextBlock heap_ctx_tx, heap_ctx_rx;
BufContextBlock *pbuf_ctx_tx = &buf_ctx_tx;
BufContextBlock *pbuf_ctx_rx = &buf_ctx_rx;
HeapContextBlock *pheap_ctx_tx = &heap_ctx_tx;
HeapContextBlock *pheap_ctx_rx = &heap_ctx_rx;
BlinkParams *params_tx, *params_rx;
TimingInfo unitGlobal;
char ethFrame[2048];



void __stdcall wpl_input_initialize_rx(void) {}
int __stdcall wpl_go_rx(void) { return 0; }
void __stdcall resetBufCtxBlock(struct _BufContextBlock *) {}
void __stdcall wpl_input_initialize_tx(void) {}
void __stdcall wpl_output_finalize_tx(void) {}
void __stdcall wpl_output_reset_tx(void) {}
int __stdcall wpl_go_tx(void) { return 0; }
TimingInfo __stdcall __ext_getTXTime(void) { return unitGlobal; }
TimingInfo __stdcall __ext_getRXTime(void) { return unitGlobal; }
int __stdcall __ext_setRAPID(char) { return 0; }
char __stdcall __ext_getRAPID(void) { return 0; }

HRESULT SoraUGetTxPacket(PACKET_HANDLE* Packet, VOID** Addr, UINT* Length, DWORD Timeout) { return 0; }
HRESULT SoraUCompleteTxPacket(PACKET_HANDLE Packet, HRESULT hResult) { return 0; }


void __stdcall mac_rx_callback();
void __stdcall mac_tx_callback();



int test_sibs()
{
	const int sib1_len = 18;
	const int sib2_len = 28;

	// We use the old AT&T SIB1&SIB2 for tests
	unsigned char sib1_1[sib1_len] = { 0x68, 0x4c, 0x42, 0x82, 0x19, 0x10, 0xf6, 0x6e, 0x82, 0x11, 0x92, 0x46, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_1[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xc8, 0xc1, 0x00, 0x00 };

	// LIME (Barbados, US SIB):
	//unsigned char sib1_2[sib1_len] = { 0x68, 0x4d, 0x0a, 0xc0, 0x19, 0x10, 0xf6, 0x6e, 0x82, 0x11, 0x92, 0x46, 0xa9, 0x80, 0x00, 0x00, 0x00, 0x00 };
	//unsigned char sib2_2[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0xbf, 0x04, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x08, 0x00, 0x00, 0x02, 0xcb, 0xf9, 0x56, 0x0f, 0x6e, 0x86, 0xbd, 0xd8, 0xc1, 0x00 };

	// Band 17, AT&T
	//unsigned char sib1_3[sib1_len] = { 0x68, 0x4c, 0x42, 0x82, 0x19, 0x10, 0xf6, 0x6e, 0x82, 0x11, 0x92, 0x46, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00 };
	//unsigned char sib2_3[sib2_len] = { 0x00, 0x00, 0x00, 0xFE, 0xBF, 0x04, 0x80, 0x00, 0x18, 0x5A, 0x7A, 0x02, 0x00, 0x08, 0x00, 0x00, 0x02, 0xCB, 0xF9, 0x56, 0x0F, 0x6E, 0x86, 0xB9, 0xC8, 0xC1, 0x00}; 

	// Band 17, TEST, 709 MHz
	unsigned char sib1_4[sib1_len] = { 0x68, 0x40, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x10, 0x09, 0x24, 0x6A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_4[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xc8, 0xc1, 0x00 };

	// Band 17, TEST, 710 MHz
	unsigned char sib1_5[sib1_len] = { 0x68, 0x40, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x10, 0x09, 0x24, 0x6A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_5[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xdc, 0xc1, 0x00 };

	// Band 17, TEST, 708 MHz
	unsigned char sib1_6[sib1_len] = { 0x68, 0x40, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x10, 0x09, 0x24, 0x6A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_6[sib2_len] = { 0x00, 0x00, 0x00, 0xfe, 0x9f, 0x84, 0x80, 0x00, 0x18, 0x5a, 0x7a, 0x02, 0x00, 0x04, 0x08, 0x00, 0xc2, 0xca, 0xd5, 0x56, 0x0f, 0x6e, 0x86, 0xb9, 0xb4, 0xc1, 0x00 };

	// Band 13, TEST, 782 MHz
	unsigned char sib1_7[sib1_len] = { 0x68, 0x40, 0x04, 0x03, 0x00, 0x01, 0x00, 0x00, 0x10, 0x09, 0x24, 0x6A, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char sib2_7[sib2_len] = { 0x00, 0x00, 0x00, 0xFE, 0xBF, 0x04, 0x80, 0x00, 0x18, 0x5A, 0x7A, 0x02, 0x00, 0x08, 0x00, 0x00, 0x02, 0xCB, 0xF9, 0x56, 0x0F, 0x6E, 0x86, 0xB5, 0x7C, 0xC1, 0x00 };

	// Band 12, AT&T
	unsigned char sib1_8[sib1_len] = { 0x68, 0x4C, 0x42, 0x82, 0x19, 0x10, 0xF6, 0x6E, 0x82, 0x11, 0x92, 0x46, 0xA5, 0x80, 0x00, 0x00};
	unsigned char sib2_8[sib2_len] = { 0x00, 0x00, 0x00, 0xFE, 0xBF, 0x04, 0x80, 0x00, 0x18, 0x5A, 0x7A, 0x02, 0x00, 0x08, 0x00, 0x00, 0x02, 0xCB, 0xF9, 0x56, 0x0F, 0x6E, 0x86, 0xB4, 0x8C, 0xC1, 0x00};

	// Band 5, AT&T
	//unsigned char sib1_9[sib1_len] = { 0x68, 0x4D, 0x0A, 0xC0, 0x19, 0x10, 0xF6, 0x6E, 0x82, 0x11, 0x92, 0x46, 0xA2, 0x00, 0x00, 0x00 };
	//unsigned char sib2_9[sib2_len] = { 0x00, 0x00, 0x00, 0xFE, 0xBF, 0x04, 0x80, 0x00, 0x18, 0x5A, 0x7A, 0x02, 0x00, 0x08, 0x00, 0x00, 0x02, 0xCB, 0xF9, 0x56, 0x0F, 0x6E, 0x86, 0xA0, 0x50, 0xC1, 0x00 };


	// Test SIB1s
	initSIBs(17, 739.0, 709.0, 1);
	if (memcmp(sib1, sib1_1, sib1_len) != 0)
	{
		printf("Error in SIB1, B17, US\n");
		return 0;
	}

	initSIBs(17, 739.0, 709.0, 0);
	if (memcmp(sib1, sib1_4, sib1_len) != 0)
	{
		printf("Error in SIB1, B17, non US\n");
		return 0;
	}

	initSIBs(13, 751.0, 782.0, 0);
	if (memcmp(sib1, sib1_7, sib1_len) != 0)
	{
		printf("Error in SIB1, B13, non-US\n");
		return 0;
	}

	initSIBs(12, 737.5, 707.5, 1);
	if (memcmp(sib1, sib1_8, sib1_len) != 0)
	{
		printf("Error in SIB1, B12, non-US\n");
		return 0;
	}



	// Test SIB2s
	initSIBs(17, 739.0, 709.0, 1);
	if (memcmp(sib2, sib2_1, sib2_len) != 0)
	{
		printf("Error in SIB2, B17, US\n");
		return 0;
	}

	initSIBs(17, 739.0, 709.0, 1);
	if (memcmp(sib2, sib2_4, sib2_len) != 0)
	{
		printf("Error in SIB2, B17, US\n");
		return 0;
	}

	initSIBs(17, 740.0, 710.0, 0);
	if (memcmp(sib2, sib2_5, sib2_len) != 0)
	{
		printf("Error in SIB2, B17, US\n");
		return 0;
	}

	initSIBs(17, 738.0, 708.0, 0);
	if (memcmp(sib2, sib2_6, sib2_len) != 0)
	{
		printf("Error in SIB2, B17, US\n");
		return 0;
	}


	printf("SIB test ok...\n");
	return 1;
}




int test_mac_tx_MCS()
{

	if (getTBSFromNRB(PDSCH_MODULATION_QPSK, 2, 144) != 144)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromNRB(PDSCH_MODULATION_QPSK, 2, 130) != 144)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromNRB(PDSCH_MODULATION_QPSK, 12, 520) != 520)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromNRB(PDSCH_MODULATION_QPSK, 12, 1800) != 1864)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromNRB(PDSCH_MODULATION_QPSK, 23, 1600) != 1608)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromNRB(PDSCH_MODULATION_QPSK, 23, 3624) != 3624)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromNRB(PDSCH_MODULATION_QPSK, 23, 5000) != -1)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}



	int modulation = -1;
	int16 NRB = -1;

	if (getTBSFromMCS(2, 380, &modulation, &NRB) != 424 || modulation != PDSCH_MODULATION_QPSK || NRB != 10)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromMCS(5, 1300, &modulation, &NRB) != 1320 || modulation != PDSCH_MODULATION_QPSK || NRB != 15)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromMCS(9, 3500, &modulation, &NRB) != 3624 || modulation != PDSCH_MODULATION_QPSK || NRB != 23)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}

	if (getTBSFromMCS(2, 3500, &modulation, &NRB) != -1)
	{
		printf("test_mac_tx_MCS error!\n");
		return 0;
	}


	return 1;
}


int test_mac_tx_helpers()
{

	for (int16 frame1 = 0; frame1 < 1024; frame1++)
	{
		int16 frame2 = (frame1 + 5) % 1024;
		int16 frame3 = (1024 + frame1 - 5) % 1024;
		int16 subframe1 = 2;
		int16 subframe2 = 4;
		int16 subframe3 = 1;
		if (compareTime(frame1, subframe1, frame2, subframe2) != 2)
		{
			printf("Error in compareTime!\n");
			return 0;
		}

		if (compareTime(frame1, subframe1, frame3, subframe3) != 1)
		{
			printf("Error in compareTime!\n");
			return 0;
		}

		if (compareTime(frame1, subframe1, frame1, subframe2) != 2)
		{
			printf("Error in compareTime!\n");
			return 0;
		}

		if (compareTime(frame1, subframe1, frame1, subframe3) != 1)
		{
			printf("Error in compareTime!\n");
			return 0;
		}

		if (compareTime(frame1, subframe1, frame1, subframe1) != 0)
		{
			printf("Error in compareTime!\n");
			return 0;
		}
	}

	// ***
	int16 frame;
	int16 subframe;
	noDataSubF = 1;
	dataSubF[0] = 4;

	frame = 0;
	subframe = 2;
	nextIPSubframe(&frame, &subframe);
	if (frame != 0 || subframe != 4)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}
	frame = 1;
	subframe = 4;
	nextIPSubframe(&frame, &subframe);
	if (frame != 2 || subframe != 4)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}
	frame = 2;
	subframe = 8;
	nextIPSubframe(&frame, &subframe);
	if (frame != 3 || subframe != 4)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}



	// ***
	noDataSubF = 3;
	dataSubF[0] = 1;
	dataSubF[1] = 4;
	dataSubF[2] = 7;

	frame = 0;
	subframe = 0;
	nextIPSubframe(&frame, &subframe);
	if (frame != 0 || subframe != 1)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}
	frame = 0;
	subframe = 1;
	nextIPSubframe(&frame, &subframe);
	if (frame != 0 || subframe != 4)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}
	frame = 0;
	subframe = 3;
	nextIPSubframe(&frame, &subframe);
	if (frame != 0 || subframe != 4)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}
	frame = 0;
	subframe = 6;
	nextIPSubframe(&frame, &subframe);
	if (frame != 0 || subframe != 7)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}
	frame = 0;
	subframe = 8;
	nextIPSubframe(&frame, &subframe);
	if (frame != 1 || subframe != 1)
	{
		printf("Error in nextIPSubframe!\n");
		return 0;
	}

	return 1;
}


int test_mac_txrx_msg()
{
	int error = 0;
	unsigned char bytes[1000];
	char str[1000];
	int len;
	int index;

	// Init TX
	buf_ctx_tx.mem_output_buf_size = 10000;
	buf_ctx_tx.mem_output_buf = malloc(buf_ctx_tx.mem_output_buf_size);
	buf_ctx_tx.mem_input_buf = malloc(buf_ctx_tx.mem_output_buf_size);

	// Init RX out queue
	size_t size = sizeof(MACULPacket);
	mac_rx_out_queue = s_ts_init(1, &size);

	#define MEM_BUF_RX_MAX_CNT		100			// Max number of packets that can be sent to MAC at once in the RX callback
	buf_ctx_rx.mem_output_buf_size = MEM_BUF_RX_MAX_CNT * sizeof(MACULPacket);
	buf_ctx_rx.mem_output_buf = (void *)malloc(buf_ctx_rx.mem_output_buf_size);
	buf_ctx_rx.chunk_output_buffer = malloc(sizeof(MACULPacket));

	unitGlobal.frame = 851;
	unitGlobal.subFrame = 2;

	MACULPacket *pkt = (MACULPacket *)buf_ctx_rx.chunk_output_buffer;
	pkt->crcOK = 1;
	pkt->frame = unitGlobal.frame;
	pkt->subframe = unitGlobal.subFrame;

	reset_state();


	// Initialize internal state (lastRRCframe, ...)
	memset(&unitGlobal, 0, sizeof(TimingInfo));
	pkt->len = 0;
	mac_rx_callback();
	mac_tx_callback();


	// Receive RRC Connection Request
	pkt->len = hex_to_bytes(pkt->data, 768, "20061F5C629BB455C60000000000000000000000000000000000") * 8;
	mac_rx_callback();
	mac_tx_callback();
	
	index = find_mac_tx_out_queue(0, 2);
	delete_mac_tx_out_queue(0, 2);
	len = hex_to_bytes(bytes, 1000, "3C20081f5C629BB455C660109B0030000B0000000000000000000000000000000000000000000000000000");
	if (memcmp(outQueue[index].pkt.data, bytes, len) != 0 || outQueue[index].pkt.len != 153 * 8)
	{
		printf("test_mac_txrx_msg failed: Receive RRC Connection Request\n");
		error = 1;
	}
	//bytes_to_hex(str, outQueue[index].pkt.data, outQueue[index].pkt.len / 8);
	//printf("Str: %s\n", str);


	// Receive RRC Connection Setup Complete (Segment 1)
	pkt->len = hex_to_bytes(pkt->data, 768, "3D010B8800002000720E82E4101220202064A8ED3005C1C00042") * 8;
	mac_rx_callback();
	mac_tx_callback();

	index = find_mac_tx_out_queue(852, 4);
	delete_mac_tx_out_queue(852, 4);
	if (index != -1)
	{
		printf("test_mac_txrx_msg failed: Receive RRC Connection Setup Complete (Segment 1)\n");
		error = 1;
	}
	//bytes_to_hex(str, outQueue[index].pkt.data, outQueue[index].pkt.len / 8);
	//printf("Str: %s\n", str);


	// Receive RRC Connection Setup Complete (Segment 2)
	pkt->len = hex_to_bytes(pkt->data, 768, "0198010437A023A24E350100422002000021020C00000001060C") * 8;
	mac_rx_callback();
	mac_tx_callback();

	index = find_mac_tx_out_queue(852, 4);
	delete_mac_tx_out_queue(852, 4);
	if (index != -1)
	{
		printf("test_mac_txrx_msg failed: Receive RRC Connection Setup Complete (Segment 2)\n");
		error = 1;
	}


	// Receive RRC Connection Setup Complete (Segment 3)
	pkt->len = hex_to_bytes(pkt->data, 768, "3D0100B00200000000001A000014012022069E314DE200000000") * 8;
	mac_rx_callback();
	mac_tx_callback();

	index = find_mac_tx_out_queue(1, 2);
	delete_mac_tx_out_queue(1, 2);
	len = hex_to_bytes(bytes, 1000, "210d21021fa000000800183aa80800000000000c");
	if (memcmp(outQueue[index].pkt.data, bytes, len) != 0 || outQueue[index].pkt.len != 153 * 8)
	{
		printf("test_mac_txrx_msg failed: Receive RRC Connection Setup Complete (Segment 3)\n");
		error = 1;
	}


	return !error;

}




int test_mac_tx_frame()
{
	int error = 0;
	unsigned char bytes[1000];
	char str[1000];
	int len;
	int index;
	MACDLPacket *DLSCHPkt; 

	// Init TX
	buf_ctx_tx.mem_output_buf_size = 10000;
	buf_ctx_tx.mem_output_buf = malloc(buf_ctx_tx.mem_output_buf_size);
	buf_ctx_tx.mem_input_buf = malloc(buf_ctx_tx.mem_output_buf_size);

	// Init RX out queue
	size_t size = sizeof(MACULPacket);
	mac_rx_out_queue = s_ts_init(1, &size);

#define MEM_BUF_RX_MAX_CNT		100			// Max number of packets that can be sent to MAC at once in the RX callback
	buf_ctx_rx.mem_output_buf_size = MEM_BUF_RX_MAX_CNT * sizeof(MACULPacket);
	buf_ctx_rx.mem_output_buf = (void *)malloc(buf_ctx_rx.mem_output_buf_size);
	buf_ctx_rx.chunk_output_buffer = malloc(sizeof(MACULPacket));

	unitGlobal.frame = 851;
	unitGlobal.subFrame = 2;

	MACULPacket *pkt = (MACULPacket *)buf_ctx_rx.chunk_output_buffer;
	pkt->crcOK = 1;
	pkt->frame = unitGlobal.frame;
	pkt->subframe = unitGlobal.subFrame;

	reset_state();
	init_mac_tx();

	// Initialize internal state (lastRRCframe, ...)
	memset(&unitGlobal, 0, sizeof(TimingInfo));
	DLSCHPkt = ((MACDLPacket *)buf_ctx_tx.mem_input_buf);

	noDataSubF = 2;
	dataSubF[0] = 4;
	dataSubF[1] = 8;


	for (int i = 0; i < 20480; i++)
	{
		if (i % 10240 == 12 || i % 10240 == 13 || i % 10240 == 14 || 
			i % 10240 == 15 || i % 10240 == 16 || i % 10240 == 17)
		{
			genIP = true;
		}
		mac_tx_callback();

		if (buf_ctx_tx.chunk_input_entries == 0 || DLSCHPkt[buf_ctx_tx.chunk_input_entries - 1].packetType != MAC_NO_PKT)
		{
			printf("Last packet in every subframe should be MAC_NO_PKT (-1)!\n");
			error = 1;
		}

		for (int j = 0; j < buf_ctx_tx.chunk_input_entries; j++)
		{
			if (DLSCHPkt[j].packetType != MAC_NO_PKT && 
				(DLSCHPkt[j].frame != unitGlobal.frame || DLSCHPkt[j].subframe != unitGlobal.subFrame))
			{
				printf("Packet (frame,subframe) doesn't match the system values!\n");
				error = 1;
			}

			if (DLSCHPkt[j].packetType == MAC_IP && unitGlobal.frame > 5)
			{
				printf("There shouldn't be any IP packets in a test scheduled after frame 5!\n");
				error = 1;
			}
		}

		// DEBUG: Print all packets
		/*
		if (buf_ctx_tx.chunk_input_entries > 1)
		{
			printf("(%d, %d): ", unitGlobal.frame, unitGlobal.subFrame);
			for (int j = 0; j < buf_ctx_tx.chunk_input_entries; j++)
			{
				printf("(%d,%d) ", DLSCHPkt[j].packetType, DLSCHPkt[j].packetSubtype);
			}
			printf("\n");
		}
		*/

		unitGlobal.subFrame++;
		if (unitGlobal.subFrame == 10)
		{
			unitGlobal.frame = (unitGlobal.frame + 1) % 1024;
			unitGlobal.subFrame = 0;
		}
	}


	return !error;
}







int test_mac_dutycycle()
{

	int error = 0;
	unsigned char bytes[1000];
	char str[1000];
	int len;
	int index;
	MACDLPacket *DLSCHPkt;

	lteu_context.onCycle = 10;
	lteu_context.offCycle = 10;


	fprintf(stdout, "Start: test duty cycle!\n");

	// Init TX
	buf_ctx_tx.mem_output_buf_size = 10000;
	buf_ctx_tx.mem_output_buf = malloc(buf_ctx_tx.mem_output_buf_size);
	buf_ctx_tx.mem_input_buf = malloc(buf_ctx_tx.mem_output_buf_size);

	// Init RX out queue
	size_t size = sizeof(MACULPacket);
	mac_rx_out_queue = s_ts_init(1, &size);

#define MEM_BUF_RX_MAX_CNT		100			// Max number of packets that can be sent to MAC at once in the RX callback
	buf_ctx_rx.mem_output_buf_size = MEM_BUF_RX_MAX_CNT * sizeof(MACULPacket);
	buf_ctx_rx.mem_output_buf = (void *)malloc(buf_ctx_rx.mem_output_buf_size);
	buf_ctx_rx.chunk_output_buffer = malloc(sizeof(MACULPacket));

	unitGlobal.frame = 0;
	unitGlobal.subFrame = 0;

	MACULPacket *pkt = (MACULPacket *)buf_ctx_rx.chunk_output_buffer;
	pkt->crcOK = 1;
	pkt->frame = unitGlobal.frame;
	pkt->subframe = unitGlobal.subFrame;

	reset_state();
	init_mac_tx();

	// Initialize internal state (lastRRCframe, ...)
	memset(&unitGlobal, 0, sizeof(TimingInfo));
	DLSCHPkt = ((MACDLPacket *)buf_ctx_tx.mem_input_buf);


	for (int i = 0; i < 10; i++)
	{
		fprintf(stdout, "frame: %d, subframe: %d\n", unitGlobal.frame, unitGlobal.subFrame);
		mac_tx_callback();

		if (buf_ctx_tx.chunk_input_entries == 0 || DLSCHPkt[buf_ctx_tx.chunk_input_entries - 1].packetType != MAC_NO_PKT)
		{
			printf("Last packet in every subframe should be MAC_NO_PKT (-1)!\n");
			error = 1;
		}

		for (int j = 0; j < buf_ctx_tx.chunk_input_entries; j++)
		{
			if (DLSCHPkt[j].packetType != MAC_NO_PKT &&
				(DLSCHPkt[j].frame != unitGlobal.frame || DLSCHPkt[j].subframe != unitGlobal.subFrame))
			{
				printf("Packet (frame,subframe) doesn't match the system values!\n");
				error = 1;
			}

			if (DLSCHPkt[j].packetSubtype == MAC_IP)
			{
				fprintf(stdout, "IP Packet at frame: %d, subframe: %d\n", unitGlobal.frame, unitGlobal.subFrame);
			}
		}

		unitGlobal.subFrame++;
		if (unitGlobal.subFrame == 10)
		{
			unitGlobal.frame = (unitGlobal.frame + 1) % 1024;
			unitGlobal.subFrame = 0;
		}
	}

	fprintf(stdout, "End: test duty cycle!\n");

	return !error;
}