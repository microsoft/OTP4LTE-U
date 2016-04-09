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
#include <sora.h>
#include "wpl_alloc.h"
#include "buf.h"

#include "mac_structs.h"
#include "debug.h"

//implement gettimeofday
#include < time.h >
#include < windows.h >

#include <stdio.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval* now);
//////////////////////////////////////////////

extern PSORA_UTHREAD_PROC User_Routines[MAX_THREADS];
// size_t sizes[MAX_THREADS];



// TODO: Think of how to use the same constants in Ziria PHY
// (e.g.through shared include file)
#define MAC_DL_SCH      0
#define MAC_UL_SCH      1
#define MAC_BCH         2
#define MAC_PCH         3
#define MAC_RACH        4


// Standard RNTI
#define P_RNTI			65534;				// Paging RNTI
#define SI_RNTI			65535;				// System Information RNTI



#define MEM_BUF_TX_MAX_CNT		100			// Max number of packets that can be sent to PHY at once in a callback
#define MEM_BUF_RX_MAX_CNT		100			// Max number of packets that can be sent to MAC at once in the RX callback




const int16 maxHARQBuf = 8;		// Max number of HARQ buffers

typedef struct {
	// One before the first available subframe to schedule the next RRC packet
	int16 lastRRCframe;
	int16 lastRRCsubFrame;
	// One before the first available subframe to schedule the next IP packet
	int16 lastIPframe;
	int16 lastIPsubFrame;
	// Last NDI and HARQ used
	int16 lastNDI;
	int16 lastHARQ;
	// Index of next empty subframe in <emptySubF>, for LTE-U off cycle
	int16 nextESF;

} MACContext;


extern MACContext mac_context;

// set_up_threads is defined in the compiler-generated code
// and returns the number of threads we set up 
extern int wpl_set_up_threads_tx(PSORA_UTHREAD_PROC *User_Routines);
extern int wpl_set_up_threads_rx(PSORA_UTHREAD_PROC *User_Routines);



// tracks bytes copied 
extern unsigned long long bytes_copied;


extern int dnlinkNRB;									// Number of RBs in dnlink to be used by for TX
extern int dnlinkMCS;									// MCS to be used for dnlink TX

#define MAC_NO_MCS		32
#define MAC_NO_NBR		50



#define LTE_NO_FRAMES	1024


// How many frames ahead at most can we schedule
#define MAX_TIME_HORIZON	20


// Stats
extern int32 rxSCH, rxIP, rxCCH, rxRACH, txSCH, txIP, txCCH;
extern int32 errIP, errFrag, errRLC, errRRC, errQue;
extern int32 RRCRecComp, RRCAttComp;
extern int32 txMCS[MAC_NO_MCS];
extern int32 txNBR[MAC_NO_NBR];


// Synchronization queues
extern ts_context *mac_rx_out_queue;



// Blink generated functions 
extern void wpl_input_initialize_tx();
extern void wpl_input_initialize_rx();
extern void wpl_output_finalize_tx();
extern void wpl_output_finalize_rx();
extern void wpl_output_reset_tx();
extern void wpl_output_reset_rx();
extern void wpl_global_init_tx(memsize_int heap_size);
extern void wpl_global_init_rx(memsize_int heap_size);
extern int wpl_go_tx();
extern int wpl_go_rx();

// Contex blocks
extern BufContextBlock buf_ctx_tx, buf_ctx_rx;
extern HeapContextBlock heap_ctx_tx, heap_ctx_rx;
extern BufContextBlock *pbuf_ctx_tx;
extern BufContextBlock *pbuf_ctx_rx;
extern HeapContextBlock *pheap_ctx_tx;
extern HeapContextBlock *pheap_ctx_rx;

// Parameters
extern BlinkParams paramsArr[2];
extern BlinkParams *params_tx, *params_rx;


// Used to assemble ethernet frames to be delivered to the OS/NDIS
#define MAX_ETH_FRAME_SIZE		2048
// Frame minus header (https://en.wikipedia.org/wiki/Ethernet_frame) - no CRC
#define MAX_ETH_PAYLOAD_SIZE	(MAX_ETH_FRAME_SIZE-14)
extern char ethFrame[MAX_ETH_FRAME_SIZE];



extern int noEmptySubF;
extern int emptySubF[10];
extern int noDataSubF;
extern int dataSubF[10];
extern int SIB1SF;
extern int SIB2SF;


typedef struct {
	int onCycle;
	int offCycle;
	int interval;
	int algorithm;
	unsigned long ctssubframes;

	ULONGLONG ttLastUpdate;
	bool updated;
	CRITICAL_SECTION critSec;
} LTEU_context;

extern LTEU_context lteu_context;


// Defined in LTE-MAC-test
#ifdef MAC_TEST
extern bool genIP;
#endif

// Utils
void invert_bits(unsigned char * arr, int len);
void* inmem_malloc(size_t size);


void init_mac(BlinkParams *params_tx, BlinkParams *params_rx);
void stop_mac();
void init_mac_tx();

DWORD WINAPI go_thread_tx(void * pParam);
DWORD WINAPI go_thread_rx(void * pParam);
DWORD WINAPI go_thread_udpsocket(void * pParam);


//void mac_tx_test(void * pParam);

void mac_tx(void * pParam);

//void mac_rx_test(void * pParam);
void mac_rx(void * pParam);


const int sib1_len = 18;
extern unsigned char sib1[sib1_len];
const int sib2_len = 28;
extern unsigned char sib2[sib2_len];

void initSIBs(int band, float DLfreq, float ULfreq, bool isUS);

void fill_SIB1(MACDLPacket *pkt, int16 frame);
void fill_SIB2(MACDLPacket *pkt, int16 frame);
void fill_RAR(MACDLPacket *pkt, int16 frame, int16 subframe);
//void fill_RRC_Conn_Setup(MACDLPacket *pkt, int16 frame, int16 subframe, unsigned char *ID);
void fill_DCI0(MACDLPacket *pkt, int16 frame, int16 subframe);
void fill_EMPTY(MACDLPacket *pkt, int16 frame, int16 subframe);
void schedule_EMPTY(MACDLPacket *pkt, int16 frame, int16 subframe);


int mac_tx_get_IP_packets(unsigned char * tx_bytes, int outlen);
int getTBSFromNRB(int modulation, int NRB, int len);
int getTBSFromMCS(int mcs, int len, int *modulation, int16 *NRB);
void updateMCSStats(int16 mcs, int16 lenRB);
int compareTime(int16 frame1, int16 subframe1, int16 frame2, int16 subframe2);
int checkTimeHorizon(int16 frame1, int16 subframe1, int16 frame2, int16 subframe2);
void nextRRCSubframe(int16 *frame, int16 *subframe);
void nextIPSubframe(int16 *frame, int16 *subframe);
void updateHARQ(int16 *lastHARQ, int16 *lastNDI, int16 maxHARQ);



int getTBSFromNRB_special(int modulation, int NRB);
