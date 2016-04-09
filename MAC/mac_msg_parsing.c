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
#include <string.h>
#include "mac.h"
#include "mac_msg_parsing.h"
#include <assert.h>
#include "debug.h"
#include "txrx_sync.h"


#define INC(cnt, bound) (((cnt)+1>(bound))?(bound):((cnt)+1))
#define INC_DELTA(cnt, delta, bound) (((cnt)+(delta)>(bound))?(bound):((cnt)+(delta)))

unsigned int udp_pkt_rcvd = 0;
unsigned int udp_pkt_err = 0;
unsigned int udp_pkt_miss = 0;


int bytes_to_hex(char* str, int strlen, unsigned char* bytes, int len)
{
	if ((strlen >> 1) - 1 < len)
	{
		len = (strlen >> 1) - 1;
	}

	if (bytes != NULL && str != NULL)
	{
		int i;
		for (i = 0; i < len; i++)
		{
			unsigned char t = bytes[i] / 16;
			if (t < 10) {
				str[2 * i] = t + '0';
			}
			else {
				str[2 * i] = t - 10 + 'a';
			}
			t = bytes[i] % 16;
			if (t < 10) {
				str[2 * i+1] = t + '0';
			}
			else {
				str[2 * i+1] = t - 10 + 'a';
			}
		}
		str[2 * i] = '\0';
		return 2 * i;
	}
	else
	{
		return -1;
	}
}


// Null terminated str
int hex_to_bytes(unsigned char *bytes, int len, char *str)
{
	if (bytes != NULL && str != NULL)
	{
		int i = 0;
		while (str[i] != '\0' && i < len/2)
		{
			unsigned char t = 0;
			for (int j = 0; j<2; j++)
			{
				int ind = 2 * i + j;
				t = t * 16;
				if (str[ind] > '9')
				{
					if (str[ind] > 'F')
					{
						t += str[ind] - 'a' + 10;
					}
					else
					{
						t += str[ind] - 'A' + 10;
					}
				}
				else
				{
					t += str[ind] - '0';
				}
			}
			bytes[i] = t;
			i++;
		}
		return i / 2;
	}
	else
	{
		return -1;
	}


}



unsigned char lcid_length(unsigned char lcid)
{
	unsigned char length[10] = { 26, 1, 27, 2, 28, 1, 29, 1, 30, 3 };
	unsigned char ind = 0;
	for (ind = 0; ind < 5 && length[2 * ind] != lcid; ind++);
	if (ind >= 5) return 0;
	return length[2 * ind + 1];
}



void print_sdus(sdu *out, int len)
{
	char str[4096];
	for (int i = 0; i < len; i++)
	{
		bytes_to_hex(str, 4096, out[i].bytes, out[i].len);
		printf("lcid: %d, size: %d, msg:%s\n", out[i].lcid, out[i].len, str);
	}

}


int mac_decode(sdu *sdus, int outlen, unsigned char * bytes, int len)
{
	range headers[100];
	int ind_headers = 0;
	int ind_sdus = 0;


	int i = 0, start = 0;
	unsigned char e = 1; // Extension field; 0 => no more headers
	while (i < len && e == 1 && ind_headers < 100)
	{
		unsigned char byte = bytes[i++];
		unsigned char lcid = byte & 0x1f;
		e = (byte >> 5) & 1;

		if (lcid == 0x1f) // padding
			continue;

		unsigned int length = lcid_length(lcid);
		if (length == 0)
		{
			if (e == 1)
			{
				byte = bytes[i++];
				length = byte & 0x7f;
				unsigned char f = (byte >> 7) & 1; // Format field
				if (f == 1)
				{
					byte = bytes[i++];
					length = (length << 8) + byte;
				}
			}
			else
				length = len - start - i; // last subheader, takes all remaining payload bytes
		}


		if (lcid <= 10) // keep SDUs, ignore Control Elements
		{
			headers[ind_headers].lcid = lcid;
			headers[ind_headers].start = min(start, len);
			headers[ind_headers].length = min(length, 2048);	// length of sdus[ind_sdus].bytes
			if (headers[ind_headers].start + headers[ind_headers].length > len)
			{
				headers[ind_headers].length = len - headers[ind_headers].start;
			}
			//ind_headers++;
			ind_headers = INC(ind_headers, 100);
		}
		start += length;
	}

	// At this point, i is the index of the first payload byte (if there is one).
	for (int j = 0; j < ind_headers; j++)
	{
		int start = 0 + i + headers[j].start;
		sdus[ind_sdus].lcid = headers[j].lcid;
		sdus[ind_sdus].len = headers[j].length;
		start = min(start, len);
		memcpy(sdus[ind_sdus].bytes, bytes + start, min(min(headers[j].length, 2048), len-start));
		ind_sdus = INC(ind_sdus, outlen);
	}

	return ind_sdus;
}



// MAC DL-SCH encode
int mac_encode(unsigned char *bytes, int bytes_len, sdu *sdus, int len)
{
	int ind_bytes = 0;
	for (int i = 0; i < len; i++)
	{
		sdu s = sdus[i];
		unsigned char e = 1;// (i < len - 1);
		unsigned char h = (e << 5) | (s.lcid & 0x1f);
		bytes[ind_bytes] = h;
		ind_bytes = INC(ind_bytes, bytes_len-1);

		if (e == 1 && s.lcid <= 10)
		{
			size_t length = s.len;
			unsigned char f = (length > 127);
			if (f == 1)
			{
				h = (f << 7) | ((length >> 8) & 0x7f);
				bytes[ind_bytes] = h;
				ind_bytes = INC(ind_bytes, bytes_len - 1);
				h = length & 0xff;
				bytes[ind_bytes] = h;
				ind_bytes = INC(ind_bytes, bytes_len - 1);
			}
			else
			{
				h = (f << 7) | (length & 0x7f);
				bytes[ind_bytes] = h;
				ind_bytes = INC(ind_bytes, bytes_len - 1);
			}
		}
	}

	if (len > 0)
	{
		bytes[ind_bytes] = 0x1f; // padding
		ind_bytes = INC(ind_bytes, bytes_len - 1);
	}
	
	for (int i = 0; i < len; i++)
	{
		int cplen = min(sdus[i].len, bytes_len - ind_bytes);
		memcpy(bytes + ind_bytes, sdus[i].bytes, cplen);
		ind_bytes += cplen;
	}

	return ind_bytes;
}




// RLC UL-SCH decode
int rlc_decode(unsigned char *out, int outlen, unsigned char lcid, unsigned char *bytes, int len, 
	unsigned short* rlc_sn, unsigned char* rlc_poll_flag, unsigned char* buffer, unsigned char* buffer_index)
{
	int i = 0;

	if (lcid == 0) // Transparent Mode
		;
	else if (lcid == 1 || lcid == 2) // Acknowledged Mode
	{
		unsigned char byte = bytes[i];
		i = INC(i, len - 1);
		unsigned char dc = (byte >> 7) & 1;
		if (dc == 1)
		{
			unsigned char rf = (byte >> 6) & 1;
			unsigned char p = (byte >> 5) & 1;
			unsigned char fi = (byte >> 3) & 3;
			unsigned char e = (byte >> 2) & 1;
			unsigned int sn = byte & 3;

			byte = bytes[i];
			i = INC(i, len - 1);
			sn = (sn << 8) + byte;
			
			*rlc_sn = sn;
			*rlc_poll_flag = p;

			assert(e == 0); // TODO
			if (e != 0)
			{
				logError("ERROR: fragment offset support not implemented yet, ignoring packet");
				errFrag++;
				return 0;
			}

			// Fragment reassembly TODO: reordering, detect missing
			bool isFirst = (fi & 2) == 0;
			bool isLast = (fi & 1) == 0;
			if (isFirst)
				*buffer_index = 0;

			memcpy(buffer + *buffer_index, bytes + i, min(len - i, 2048 - *buffer_index));
			*buffer_index = min(2048, *buffer_index + len - i);

			if (isLast)
			{
				int cplen = min(*buffer_index, outlen);
				memcpy(out, buffer, cplen);
				return cplen;
			}
			else
				return 0; // prevent packet from propagating if not last fragment
		}
		else
		{
			// STATUS - ignored
			unsigned int ack_sn = (byte & 0xf);

			byte = bytes[i++];
			ack_sn = (ack_sn << 6) + ((byte >> 2) & 0x3f);
			unsigned char e1 = (byte >> 1) & 1;
			if (e1)
			{
				unsigned short nack_sn = ((byte << 9) | (bytes[i] << 1) | (bytes[i + 1] >> 7)) & 0x3ff;
				e1 = (bytes[i + 1] >> 6) & 1;
				assert(e1 == 0); // TODO
				if (debug_level > 0)
				{
					printf("RLC status report on LCID %d: seen SN %d, but missing SN %d (maybe others)\n", lcid, ack_sn - 1, nack_sn);
				}
				errRLC++;
			}
			return 0; // prevent packet from propagating as we may not have handled all of status report
		}
	}
	else // Unacknowledged Mode
	{
		unsigned char byte = bytes[i];
		i = INC(i, len - 1);
		unsigned char fi = (byte >> 6) & 3;
		unsigned char e = (byte >> 5) & 1;
		unsigned char sn = byte & 0x1f;

		assert(e == 0); // TODO
		if (e != 0)
		{
			logError("ERROR: fragment offset support not implemented yet, ignoring packet");
			errFrag++;
			return 0;
		}

		// Fragment reassembly TODO: reordering, detect missing
		bool isFirst = (fi & 2) == 0;
		bool isLast = (fi & 1) == 0;
		if (isFirst)
			*buffer_index = 0;

		memcpy(buffer + *buffer_index, bytes + i, min(len - i, 2048 - *buffer_index));
		*buffer_index = min(2048, *buffer_index + len - i);

		if (isLast)
		{
			int cplen = min(*buffer_index, outlen);
			memcpy(out, buffer, cplen);
			return cplen;
		}
		else
			return 0; // prevent packet from propagating if not last fragment
	}

	int cplen = min(len - i, outlen);
	memcpy(out, bytes + i, cplen);

	return cplen;
}



// RLC DL-SCH encode
int rlc_encode(unsigned char *mac_bytes, int outlen, unsigned char lcid, unsigned int *sn, unsigned char* bytes, int len)
{
	int ind_mac = 0;
	if (lcid == 0 || lcid >= 28) // Transparent Mode
		;
	else if (lcid == 1 || lcid == 2) // Acknowledged Mode
	{
		unsigned char dc = 1;
		unsigned char p = 1;
		unsigned char h = (dc << 7) | (p << 5);
		h = h | (((*sn) >> 8) & 3);
		mac_bytes[ind_mac] = h;
		ind_mac = INC(ind_mac, outlen - 1);
		h = (*sn) & 0xff;
		mac_bytes[ind_mac] = h;
		ind_mac = INC(ind_mac, outlen - 1);
		(*sn)++;
	}
	else // Unacknowledged Mode
	{
		unsigned char h = (*sn & 0x1f); // other fields all zero
		mac_bytes[ind_mac] = h;
		ind_mac = INC(ind_mac, outlen - 1);
		(*sn)++;
	}

	int cp_len = min(len, outlen - ind_mac);
	memcpy(mac_bytes + ind_mac, bytes, cp_len);

	return ind_mac + cp_len;
}



// PDCP UL-SCH decode
int pdcp_decode(unsigned char* out, int outlen, unsigned char lcid, unsigned char *bytes, int len)
{
	int i = 0, j = 0;

	if (len == 0) // ignore zero-length packet
		return 0;

	if (lcid == 0) // Transparent Mode
		;
	else if (lcid == 1 || lcid == 2) // Control plane
	{
		unsigned char byte = bytes[i++];
		unsigned char sn = byte & 0x1f;
		j = 4; // ignore MAC-I
	}
	else // User plane
	{
		unsigned char byte = bytes[i++];
		unsigned char dc = (byte >> 7) & 1;
		if (dc == 1)
		{
			unsigned char sn = byte & 0x7f;
		}
		else
		{
			i = len; // ignore control packet
		}
	}

	int cp_len = min(len, len - j - i);
	memcpy(out, bytes + i, cp_len);
	return cp_len;
}


// PDCP DL-SCH encode
int pdcp_encode(unsigned char * rlc_bytes, int outlen, unsigned char lcid, unsigned int *sn, unsigned char *bytes, int len)
{
	int out_len = 0;
	if (lcid == 0 || lcid >= 28) // Transparent Mode
	{
		memcpy(rlc_bytes, bytes, min(len, outlen));
		out_len = INC_DELTA(out_len, len, outlen);
	}
	else if (lcid == 1 || lcid == 2) // Control plane
	{
		unsigned char h = ((*sn) & 0x1f); // other fields all zero
		rlc_bytes[out_len] = h;
		out_len = INC(out_len, outlen);

		(*sn)++;
		memcpy(rlc_bytes + out_len, bytes, min(len, outlen - out_len));
		out_len = INC_DELTA(out_len, len, outlen);
		memset(rlc_bytes + out_len, 0, min(4, outlen - out_len));  // MAC-I
		out_len = INC_DELTA(out_len, 4, outlen);
	}
	else // User plane
	{
		unsigned char dc = 1;
		unsigned char h = (dc << 7) | ((*sn) & 0x7f);
		rlc_bytes[out_len] = h;
		out_len = INC(out_len, outlen);
		(*sn)++;

		int cp_len = min(outlen - out_len, len);
		memcpy(rlc_bytes + out_len, bytes, cp_len);
		out_len += cp_len;
	}

	return out_len;
}



int ping(unsigned char * bytes, int outlen, unsigned int source_ip, unsigned int dest_ip)
{
	//60 bytes. IP checksum: sum shorts, add carries, flip bits
	//IP:        45 00 00 3C E1 0C 00 00 20 01 
	//*checksum:  7B B4
	//*source IP: 0C 4A 65 BE
	//*dest IP:   D0 88 FB 6F
	//ICMP: 08 00 96 5B 08 00 AF 00
	//payload: 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68 69 a..wa..i

	// IP header
	int len;
	len = hex_to_bytes(bytes, outlen, "4500003ce10c00002001"); // IP header
	len += hex_to_bytes(bytes + len, outlen-len, "0000"); // checksum - to be overwritten

	bytes[len] = (unsigned char)(source_ip >> 24);
	len = INC(len, outlen);
	bytes[len] = (unsigned char)(source_ip >> 16);
	len = INC(len, outlen);
	bytes[len] = (unsigned char)(source_ip >> 8);
	len = INC(len, outlen);
	bytes[len] = (unsigned char)(source_ip);
	len = INC(len, outlen);
	bytes[len] = (unsigned char)(dest_ip >> 24);
	len = INC(len, outlen);
	bytes[len] = (unsigned char)(dest_ip >> 16);
	len = INC(len, outlen);
	bytes[len] = (unsigned char)(dest_ip >> 8);
	len = INC(len, outlen);
	bytes[len] = (unsigned char)(dest_ip);
	len = INC(len, outlen);

	// IP header checksum
	unsigned int checksum = 0;
	for (int i = 0; i < len / 2; i++)
	{
		unsigned short s = (bytes[2*i] << 8) | bytes[2*i + 1];
		checksum += s;
	}
	while (checksum >> 16) checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = ~checksum;
	if (outlen > 11)
	{
		bytes[10] = checksum >> 8;
		bytes[11] = checksum & 0xff;
	}

	// ICMP packet
	len += hex_to_bytes(bytes + len, outlen - len, "0800965b0800af00"); // ICMP header
	len += hex_to_bytes(bytes + len, outlen - len, "6162636465666768696a6b6c6d6e6f7071727374757677616263646566676869");
	return len;
}


// DL response to UL RRC
int rrc(sdu *sdus, int outlen, unsigned char lcid, unsigned char * bytes, int len)
{
	if (len == 0) // ignore zero-length packet
		return 0;

	TimingInfo txTime = __ext_getTXTime();
	TimingInfo rxTime = __ext_getRXTime();


	int len_sdus = 0;
	if (lcid == 0) // UL-CCCH-Message
	{
		logRRC(DEBUG_LOG_UP, rxTime, 0, "RRCConnectionRequest", bytes, len);
		reset_state();				// this is a new connection attempt

		assert(bytes[0] >> 6 == 1); // RRCConnectionRequest
		sdus[len_sdus].lcid = 28;	// Contention Resolution ID control element
		memcpy(sdus[len_sdus].bytes, bytes, min(len, 2048));
		sdus[len_sdus].len = len;
		len_sdus = INC(len_sdus, outlen);

		logRRC(DEBUG_LOG_DN, txTime, 0, "RRCConnectionSetup");
		sdus[len_sdus].lcid = 0;	// RRCConnectionSetup
		sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "60109b0030000b00");
		len_sdus = INC(len_sdus, outlen);
	}
	else if (lcid == 1 || lcid == 2) // UL-DCCH-Message
	{
		unsigned char c1 = (bytes[0] >> 3);
		if (c1 == 2) // RRCConnectionReconfigurationComplete
		{
			logRRC(DEBUG_LOG_UP, rxTime, lcid, "RRCConnectionReconfigurationComplete", bytes, len);
			RRCRecComp++;
			// TODO: check for embedded Attach complete
			// BR: Not always necessary. If a connection is temporary broken, UE is still attached and resync stops at RRCConnectionReconfigurationComplete
		}
		else if (c1 == 4) // RRCConnectionSetupComplete
		{
			logRRC(DEBUG_LOG_UP, rxTime, lcid, "RRCConnectionSetupComplete", bytes, len);
			logRRC(DEBUG_LOG_DN, txTime, 1, "Identity request");
			sdus[len_sdus].lcid = 1;
			sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "0800183aa808");
			len_sdus = INC(len_sdus, outlen);
		}
		else if (c1 == 5) // RRC SecurityModeComplete
		{
			logRRC(DEBUG_LOG_UP, rxTime, lcid, "RRC SecurityModeComplete", bytes, len);
			logRRC(DEBUG_LOG_DN, txTime, 1, "RRCConnectionReconfiguration");
			sdus[len_sdus].lcid = 1;
			sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "200603A93800000000103A101700300007888000080242900E08083960331B6BB9A981806B937B4323296B9B1B43BB0B93D01B1B7B68336B7319818188336B1B19818188233B839398280E054008113874040108501808005408360540080A805FB0078880000001A21D43E0980788800009182FA1A21D43E313E0FA81484A064C0284218300"); // was 200600A03A100F0000005A9006080000280E05400810000303E0F281784000
			len_sdus = INC(len_sdus, outlen);
		}
		else if (c1 == 9) // ULInformationTransfer
		{
			unsigned char lenDedicatedInfoNAS = (((bytes[1] << 8) | bytes[2]) >> 5) & 0x7f;
			unsigned char securityHeaderType = (bytes[2] >> 1) & 0xf;
			unsigned char messageType = (((bytes[3] << 8) | bytes[4]) >> 5);
			if (securityHeaderType > 0 && securityHeaderType < 5) // Security protected NAS message
				messageType = (((bytes[9] << 8) | bytes[10]) >> 5);

			if (messageType == 67) // Attach complete
			{
				// We should be ok to send IP traffic on LCID 3 at this point. Try a ping.
				logRRC(DEBUG_LOG_UP, rxTime, lcid, "Attach complete", bytes, len);
				RRCAttComp++;
				//logRRC(DEBUG_LOG_DN, txTime, 3, "ping");
				//sdus[len_sdus].lcid = 3;
				//sdus[len_sdus].len = ping(sdus[len_sdus].bytes, 2048, 0xc0a80101, 0xc0a80102);
				//len_sdus = INC(len_sdus, outlen);
			}
			else if (messageType == 69) // Detach request
			{
				logRRC(DEBUG_LOG_UP, rxTime, lcid, "Detach request", bytes, len);
				logRRC(DEBUG_LOG_DN, txTime, lcid, "Detach accept");
				sdus[len_sdus].lcid = lcid;
				sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "0800413800000000003A30");
				len_sdus = INC(len_sdus, outlen);

				logRRC(DEBUG_LOG_DN, txTime, 1, "RRCConnectionRelease");
				sdus[len_sdus].lcid = 1;
				sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "2802");
				len_sdus = INC(len_sdus, outlen);
			}
			else if (messageType == 83) // Authentication response
			{
				logRRC(DEBUG_LOG_UP, rxTime, lcid, "Authentication response", bytes, len);
				logRRC(DEBUG_LOG_DN, txTime, 1, "NAS Security mode command");
				sdus[len_sdus].lcid = 1;
				// TODO: respond with identical SecurityCapabilities
				//sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "080081B800000000003AE80000270706060608"); // Bozidar's dongle
				sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "080081B800000000003AE80000170706080000"); // Doug's dongle
				len_sdus = INC(len_sdus, outlen);
			}
			else if (messageType == 86) // Identity response
			{
				logRRC(DEBUG_LOG_UP, rxTime, lcid, "Identity response", bytes, len);
				logRRC(DEBUG_LOG_DN, txTime, 1, "Authentication request");
				sdus[len_sdus].lcid = 1;
				sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "0801203A9002AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB082B28A829A92E40002AAA2BAB28A8E9A90");
				len_sdus = INC(len_sdus, outlen);
			}
			else if (messageType == 94) // NAS Security mode complete
			{
				logRRC(DEBUG_LOG_UP, rxTime, lcid, "NAS Security mode complete", bytes, len);
				logRRC(DEBUG_LOG_DN, txTime, 1, "RRC SecurityModeCommmand");
				sdus[len_sdus].lcid = 1;
				sdus[len_sdus].len = hex_to_bytes(sdus[len_sdus].bytes, 2048, "300000");
				len_sdus = INC(len_sdus, outlen);
			}
			else if (messageType == 95) // NAS Security mode reject
			{
				unsigned char cause = (((bytes[4] << 8) | bytes[5]) >> 5) & 0xFF;
				char str[1000];
				sprintf(str, "NAS Security mode reject (cause = %d)", cause);
				logRRC(DEBUG_LOG_UP, rxTime, lcid, str, bytes, len);
				errRRC++;
				//logRRC(DEBUG_LOG_UP, rxTime, lcid, "NAS Security mode reject; cause = ", &cause, 1);
			}
			else
			{
				if (debugPrint(DEBUG_PRINT_RRC))
				{
					char str[1000];
					sprintf(str, "UNKNOWN ULInformationTransfer message type (type = %d)", messageType);
					logRRC(DEBUG_LOG_UP, rxTime, lcid, str, bytes, len);
					errRRC++;
				}
			}
		}
		else
		{
			if (debugPrint(DEBUG_PRINT_RRC))
			{
				char str[1000];
				sprintf(str, "UNKNOWN UL-DCCH-Message (type = %d)", c1);
				logRRC(DEBUG_LOG_UP, rxTime, lcid, str, bytes, len);
				errRRC++;
			}
		}
	}
	else // IP traffic
	{
		rxIP++;
		logIP(DEBUG_LOG_UP, rxTime, lcid, "IP", bytes, len);
		if (test_IP)
		{
			int tx_len = ip(bytes, len, sdus[len_sdus].bytes, 2048);
			if (tx_len > 0)
			{
				logIP(DEBUG_LOG_DN, txTime, lcid, "IP", sdus[len_sdus].bytes, tx_len);
				sdus[len_sdus].lcid = lcid;
				sdus[len_sdus].len = tx_len;
				len_sdus = INC(len_sdus, outlen);
			}
		} 
		else 
		{
			// Sanity check - do not deliver too big packets
			if (len < MAX_ETH_PAYLOAD_SIZE)
			{
				// Send to NDIS
				memcpy((void *)(ethFrame + 14), (void *)bytes, min(len, 2048-14) *sizeof(char));
				// TODO: Calculate CRC
				HRESULT hResult = SoraUIndicateRxPacket((unsigned char *) ethFrame, len+14);

				// DEBUG
				//char str[4096];
				//bytes_to_hex(str, 4096, (unsigned char *)ethFrame, len + 14);
				//printf("IP delivered to NDIS: size: %d, msg:%s\n", len+14, str);

				if (hResult != S_OK)
				{
					logError("IP packet delivery to NDIS failed!");
					errIP++;
				}
			}
			else
			{
				//printf("Packet too big (%d B), not delivered to NDIS!\n", len);
				logError("Packet too big, not delivered to NDIS!");
				errIP++;
			}
		}
	}

	return len_sdus;
}

int ignore(char * reason, unsigned char * bytes, int len)
{
	if (debug_level > 0)
	{
		char str[1000] = "";
		bytes_to_hex(str, 1000, bytes, len);
		printf("Ignoring packet: %s\nReason: %s\n", str, reason);
	}	
	return 0;
}

unsigned short checksum(unsigned char * bytes, unsigned int len)
{
	unsigned int checksum = 0;
	for (int i = 0; i < len / 2; i++)
	{
		unsigned short s = (bytes[2 * i] << 8) | bytes[2 * i + 1];
		checksum += s;
	}
	while (checksum >> 16) checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = ~checksum;
	return checksum;
}

typedef struct
{
	uint8 versionAndHeaderLength;
	uint8 typeOfService;
	uint16 totalLength;
	uint16 identification;
	uint16 fragmentation;
	uint8 timeToLive;
	uint8 protocol;
	uint16 checksum;
	uint32 sourceIP;
	uint32 destIP;
} ip_header;

#pragma comment(lib, "ws2_32.lib") // ntohs, ntohl, htons, htonl

int ip(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen)
{
#define BAIL(reason) { ignore((reason), rx_bytes, rx_len); return 0; }

	// Decode IP header.
	if (rx_len < 20)
		BAIL("Not enough bytes for IP header");

	ip_header* header = (ip_header*)rx_bytes;
	uint8 version = header->versionAndHeaderLength >> 4;
	uint8 headerLengthBytes = 4 * (header->versionAndHeaderLength & 0xf);
	header->totalLength = ntohs(header->totalLength);
	header->identification = ntohs(header->identification);
	header->fragmentation = ntohs(header->fragmentation);
	header->checksum = ntohs(header->checksum);
	header->sourceIP = ntohl(header->sourceIP);
	header->destIP = ntohl(header->destIP);

	if (version != 4)
		BAIL("Only support IPv4");
	if (headerLengthBytes < 20)
		BAIL("Header length below minimum");
	if (headerLengthBytes > rx_len)
		BAIL("Header length exceeds available bytes");
	if (header->totalLength > rx_len)
	{
		if (debug_level > 0)
		{
			printf("header->totalLength=%d, rx_len=%d, ", header->totalLength, rx_len);
		}
		BAIL("Total length exceeds available bytes");
	}
	unsigned char* payload = rx_bytes + headerLengthBytes;

	// TODO: validate other fields, e.g. checksum
	// TODO: what if rx_len > totalLength? Could we get more than one IP packet?

	// Handle payload based on protocol.
	unsigned int tx_payload_len = 0;
	unsigned char* tx_payload = tx_bytes + headerLengthBytes;

	switch (header->protocol)
	{
		case 1: tx_payload_len = icmp(payload, header->totalLength - headerLengthBytes, tx_payload, outlen - headerLengthBytes); break;
		//case 2: tx_payload_len = igmp(payload, header->totalLength - headerLengthBytes, tx_payload, outlen - headerLengthBytes); break;
		case 17: 
			tx_payload_len = udp(payload, header->totalLength - headerLengthBytes, tx_payload, outlen - headerLengthBytes); 
			// Nothing to send here
			return 0;
			break;
		default: BAIL("Unknown protocol");
	}

	// Encode IP header.
	unsigned int tx_len = headerLengthBytes + tx_payload_len;
	if (outlen < tx_len)
		BAIL("Transmit packet size exceeds available bytes");

	memcpy(tx_bytes, rx_bytes, min(headerLengthBytes, outlen));
	ip_header* tx_header = (ip_header*)tx_bytes;
	tx_header->totalLength = htons(tx_len);
	tx_header->identification = htons(header->identification);
	tx_header->fragmentation = htons(header->fragmentation);
	tx_header->sourceIP = htonl(header->destIP);
	tx_header->destIP = htonl(header->sourceIP);
	tx_header->checksum = 0; // important - don't include old checksum in new calculation
	tx_header->checksum = htons(checksum(tx_bytes, tx_len));

	return tx_len;
}

typedef struct
{
	uint8 type;
	uint8 code;
	uint16 checksum;
	uint32 restOfHeader;
} icmp_header;

int icmp(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen)
{
#define BAIL(reason) { ignore((reason), rx_bytes, rx_len); return 0; }

	// Decode ICMP header.
	if (rx_len < 8)
		BAIL("Not enough bytes for ICMP header");

	icmp_header* header = (icmp_header*)rx_bytes;
	header->checksum = ntohs(header->checksum);
	header->restOfHeader = ntohl(header->restOfHeader);

	// Build response based on type.
	switch (header->type)
	{
		case 0: BAIL("Ping response");
		case 8: // Ping request
		{
			if (outlen < rx_len) // tx and rx length will be the same
				BAIL("Transmit ICMP size exceeds available bytes");

			// DEBUG print of ICMP/Ping seq no
			uint16 *id = (uint16*)(&(header->restOfHeader));
			uint16 *seq = (uint16*)(&(header->restOfHeader)) + 1;
			printf("Ping request, id=%d, seqno=%d\n", *id, *seq);

			memcpy(tx_bytes, rx_bytes, min(rx_len, outlen));
			icmp_header* tx_header = (icmp_header*)tx_bytes;
			tx_header->type = 0; // ping response
			tx_header->restOfHeader = htonl(header->restOfHeader);
			tx_header->checksum = 0; // important - don't include old checksum in new calculation
			tx_header->checksum = htons(checksum(tx_bytes, rx_len));
			return rx_len;
		}
		default: BAIL("Unknown ICMP type");
	}
}


typedef struct
{
	uint16 srcPort;
	uint16 dstPort;
	uint16 length;
	uint16 checksum;
} udp_header;


// Maasures stats assuming input from LTE/Utils/NetTest/client.py
int udp(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen)
{
#define BAIL(reason) { ignore((reason), rx_bytes, rx_len); return 0; }

	static int old_index = 0;

	// Decode ICMP header.
	if (rx_len < 8)
		BAIL("Not enough bytes for UDP header");

	udp_header* header = (udp_header*)rx_bytes;
	header->srcPort = ntohs(header->srcPort);
	header->dstPort = ntohs(header->dstPort);
	header->checksum = ntohs(header->checksum);
	// Not sure why sizeof(header) == 4 here instead of 8, so I hardcode 8
	if ((int)ntohs(header->length) - 8 <= 0)
	{
		BAIL("No/wrong payload");
	}
	else
	{
		header->length = ntohs(header->length) - 8;
	}

	unsigned char * payload = (unsigned char *)header + 8;

	int i = 1;
	int index = payload[0];
	bool error = (header->length > rx_len - 8);
	while (!error && i < header->length && i < rx_len - 8)
	{
		error = (payload[i] != index);
		i++;
	}

	// DEBUG
	if (debug_level > 0 && index <= old_index)
	{
		char str[4096];
		int l = bytes_to_hex(str, 4096, payload, header->length);
		//printf("%d:%d, len:%d, index=%d, error=%d, i=%d, ", header->srcPort, header->dstPort, header->length, index, error, i);
		printf("Weird index: old=%d, new=%d, msg: ", index, old_index);
		for (i = 0; i < header->length; i++)
		{
			printf("%d:", payload[i]);
		}
		printf("\n");
	}

	if (error) {
		udp_pkt_err++;
		printf("EE ");
	}
	else {
		udp_pkt_rcvd++;
		udp_pkt_miss += ((128 + index - old_index) % 128) - 1;
		old_index = index;
		printf("%d ", index);
	}

	return 0;
}



// State that must persist between calls to end_to_end. 
const int MAX_LCID_COUNT = 4;
unsigned char rlc_sdu_buffer[MAX_LCID_COUNT][MAX_MAC_MSG_SIZE]; // TODO: actual max size
unsigned char rlc_sdu_buffer_index[MAX_LCID_COUNT];
unsigned int rlc_tx_sn[MAX_LCID_COUNT];

void reset_state() // call this at top of tests that use end_to_end
{
	memset(rlc_sdu_buffer, 0, sizeof(rlc_sdu_buffer));
	memset(rlc_sdu_buffer_index, 0, sizeof(rlc_sdu_buffer_index));
	memset(rlc_tx_sn, 0, sizeof(rlc_tx_sn));
}

int end_to_end(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen)
{
	// Briefly:
	// mac_decode(bytes[]) -> sdus[]
	// pdcp/rlc_decode(sdu) -> sdu
	// rrc(sdu) -> sdus[]
	// pdcp/rlc_encode(sdu) -> sdu
	// mac_encode(sdus[]) -> bytes[]
	int num_rx_sdus, num_rrc_sdus, num_tx_sdus = 0, pdcp_len, rrc_len, rlc_len;
#define SDU_SIZE	64
	sdu rx_sdus[SDU_SIZE], tx_sdus[SDU_SIZE]; // TODO: actual max sizes
	unsigned char pdcp_bytes[MAX_MAC_MSG_SIZE], rrc_bytes[MAX_MAC_MSG_SIZE], rlc_bytes[MAX_MAC_MSG_SIZE];

	// State that is captured on the receive side and later reused on the transmit side.
	unsigned short rlc_rx_sn = 0;
	unsigned char rlc_poll_flag = 0;

	num_rx_sdus = mac_decode(rx_sdus, SDU_SIZE, rx_bytes, rx_len);
	for (int i = 0; i < num_rx_sdus; i++)
	{
		int pdcp_len = rlc_decode(pdcp_bytes, 2048, rx_sdus[i].lcid, rx_sdus[i].bytes, rx_sdus[i].len, 
			&rlc_rx_sn, &rlc_poll_flag, rlc_sdu_buffer[rx_sdus[i].lcid], &rlc_sdu_buffer_index[rx_sdus[i].lcid]);
		int rrc_len = pdcp_decode(rrc_bytes, 2048, rx_sdus[i].lcid, pdcp_bytes, pdcp_len);
		num_rrc_sdus = rrc(tx_sdus + num_tx_sdus, SDU_SIZE - num_tx_sdus, rx_sdus[i].lcid, rrc_bytes, rrc_len);

		for (int j = num_tx_sdus; j < num_tx_sdus + num_rrc_sdus; j++) // RRC may generate multiple TX SDUs for 1 RX SDU
		{
			unsigned int pdcp_sn = 0; // TODO: what to do here?
			pdcp_len = pdcp_encode(pdcp_bytes, 2048, tx_sdus[j].lcid, &pdcp_sn, tx_sdus[j].bytes, tx_sdus[j].len);
			rlc_len = rlc_encode(rlc_bytes, 2048, tx_sdus[j].lcid, &rlc_tx_sn[tx_sdus[j].lcid], pdcp_bytes, pdcp_len);
			memcpy(tx_sdus[j].bytes, rlc_bytes, min(rlc_len, 2048));
			tx_sdus[j].len = min(rlc_len, 2048);
		}
		num_tx_sdus += num_rrc_sdus;

		if (rlc_poll_flag) // send RLC Status Report
		{ // INFO: alternative way to include Status Report would be to append to data packet in rlc_encode
			unsigned short report = ((rlc_rx_sn + 1) & 0x3ff) << 2;
			tx_sdus[num_tx_sdus].lcid = rx_sdus[i].lcid;
			tx_sdus[num_tx_sdus].bytes[0] = report >> 8;
			tx_sdus[num_tx_sdus].bytes[1] = report;
			tx_sdus[num_tx_sdus].len = 2;
			num_tx_sdus++;
		}
	}

	return mac_encode(tx_bytes, outlen, tx_sdus, num_tx_sdus);
}
