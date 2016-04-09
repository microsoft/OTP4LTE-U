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
#include <types.h>
#include "txrx_sync.h"
#include "mac.h"

// DEBUG LEVELS
// PHY(L1)
#define DEBUG_PRINT_ENERGY      1
#define DEBUG_PRINT_PRACH       2
#define DEBUG_PRINT_PDCCH       4
#define DEBUG_PRINT_PDSCH       8
#define DEBUG_PRINT_PUCCH       16
#define DEBUG_PRINT_PUSCH       32
// MAC(L2)
#define DEBUG_PRINT_DLCCH       64
#define DEBUG_PRINT_DLSCH       128
#define DEBUG_PRINT_ULCCH       256
#define DEBUG_PRINT_ULSCH       512
// RRC(L3)
#define DEBUG_PRINT_RRC         1024
#define DEBUG_PRINT_IP_HEAD		2048
#define DEBUG_PRINT_RRC_DATA    4096
#define DEBUG_PRINT_IP_DATA		8192
// Radio
#define DEBUG_PRINT_RADIO		16384


// debug_level == 0 means no messages at all!
extern int32 debug_level;
extern unsigned char __ext_debugPrint(int32 level);
inline bool debugPrint(int32 level) { return debug_level & level; }

int bytes_to_hex(char* str, int strlen, unsigned char* bytes, int len);

#define DEBUG_LOG_UP 1
#define DEBUG_LOG_DN 0

inline void log(bool uplink, TimingInfo time, unsigned char lcid, char* msg, int32 debug_level, bool ip, unsigned char* bytes, unsigned int len)
{
	if (debugPrint(debug_level))
	{
		if (!debugPrint(DEBUG_PRINT_IP_DATA) && len >= 20 && ip)
		{
			int s = 12;
			printf("%s %4d.%d [%d] %s %d.%d.%d.%d -> %d.%d.%d.%d,", uplink ? "^" : "v", time.frame, time.subFrame, lcid, msg, 
				bytes[s], bytes[s + 1], bytes[s + 2], bytes[s + 3], bytes[s+4], bytes[s+5], bytes[s+6], bytes[s+7]);
			if (len >= 28) printf("payload: %02x...\n", bytes[28]);
			else printf("\n");
		}
		else
		{
			char str[1000] = "";
			bytes_to_hex(str, 1000, bytes, len);
			printf("%s %4d.%d [%d] %s %s\n", uplink ? "^" : "v", time.frame, time.subFrame, lcid, msg, str);
		}
	}
}

inline void logRRC(bool uplink, TimingInfo time, unsigned char lcid, char* msg, unsigned char* bytes = 0, unsigned int len = 0)
{
	if (debugPrint(DEBUG_PRINT_RRC_DATA))
	{
		log(uplink, time, lcid, msg, DEBUG_PRINT_RRC, false, bytes, len);
	}
	else
	{
		log(uplink, time, lcid, msg, DEBUG_PRINT_RRC, false, NULL, 0);
	}
}

inline void logIP(bool uplink, TimingInfo time, unsigned char lcid, char* msg, unsigned char* bytes = 0, unsigned int len = 0)
{
	log(uplink, time, lcid, msg, DEBUG_PRINT_IP_DATA | DEBUG_PRINT_IP_HEAD, true, bytes, len);
}


inline void logDLSCH(MACDLPacket newPkt, int orig_len, char *str)
{
	if (debugPrint(DEBUG_PRINT_DLSCH))
	{
		printf("vvv TX %s Pkt at (%d, %d), %dB (pre padding %dB), lenRB=%d, NDI=%d: ", str, 
			newPkt.frame, newPkt.subframe, newPkt.len >> 3, orig_len >> 3, newPkt.lenRB, newPkt.NDI);
		//for (int i = 0; i < newPkt.len >> 3; i++) printf("%02X", newPkt.data[i]);
		printf("\n");
	}

}

inline void logError(char * str)
{
	if (debug_level > 0)
	{
		printf("%s\n", str);
	}
}