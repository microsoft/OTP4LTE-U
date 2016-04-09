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
#include "Tests\c\unit_tests.h"
#include <types.h>
#include "debug.h"

// Setting debug level
int32 debug_level = DEBUG_PRINT_RRC | DEBUG_PRINT_IP_DATA;				// Default - only RRC and IP messages
unsigned char __ext_debugPrint(int32 level) { return (unsigned char)(debug_level & level); }

bool test_IP = false;
int LTEBand;
int dnlinkNRB;									// Number of RBs in dnlink to be used by for TX
int dnlinkMCS;									// MCS to be used for dnlink TX
int noEmptySubF = 0;								// Subframes in LTE-U off cycle (no data, RS, anything)
int emptySubF[10];
int noDataSubF = 0;									// Subframes to be used for IP/data transmissions
int dataSubF[10];
int SIB1SF = 5;
int SIB2SF = 0;
LTEU_context lteu_context;

bool genIP = false;

// Global logs
int32 rxSCH = 0, rxIP = 0, rxCCH = 0, rxRACH = 0, txSCH = 0, txIP = 0, txCCH = 0;
int32 errIP = 0, errFrag = 0, errRLC = 0, errRRC = 0, errQue = 0;;
int32 RRCRecComp = 0, RRCAttComp = 0;
int32 txMCS[MAC_NO_MCS];
int32 txNBR[MAC_NO_NBR];


long SoraUIndicateRxPacket(UCHAR* Buffer, ULONG BufferLength) { return 0; }



int __cdecl main(int argc, char **argv)
{
	int ok = 1;

	ok = test_sibs() && ok;

	ok = test_mac_txrx_msg() && ok;

	ok = test_mac_tx_frame() && ok;

	ok = test_mac_msg_parsing() && ok;

	ok = test_mac_tx_out_queue() && ok;

	ok = test_mac_tx_MCS() && ok;

	ok = test_mac_tx_helpers() && ok;

	ok = test_mac_dutycycle() && ok;

	if (!ok)
	{
		printf("\n\n\n*** ERRORS DETECTED! ***\n");
		return -1;
	}

	printf("\n\n\n*** TESTS PASSED! ***\n");
	return 0;
}




