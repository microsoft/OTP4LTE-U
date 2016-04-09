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
#define MAX_MAC_MSG_SIZE		2048
typedef struct { unsigned char lcid; unsigned int len;  unsigned char bytes[MAX_MAC_MSG_SIZE]; } sdu;
typedef struct { unsigned char lcid; unsigned int start; unsigned int length; } range;

// Setting test_IP
extern bool test_IP;


int bytes_to_hex(char* str, int strlen, unsigned char* bytes, int len);
int hex_to_bytes(unsigned char *bytes, int len, char *str);
void print_sdus(sdu *out, int len);

int mac_decode(sdu *sdus, int outlen, unsigned char * bytes, int len);
int mac_encode(unsigned char *bytes, int bytes_len, sdu *sdus, int len);
int rlc_decode(unsigned char *out, int outlen, unsigned char lcid, unsigned char *bytes, int len, unsigned short* rlc_sn, unsigned char* rlc_poll_flag, unsigned char* buffer, unsigned char* buffer_index);
int rlc_encode(unsigned char *mac_bytes, int outlen, unsigned char lcid, unsigned int *sn, unsigned char* bytes, int len);
int pdcp_decode(unsigned char* out, int outlen, unsigned char lcid, unsigned char *bytes, int len);
int pdcp_encode(unsigned char * rlc_bytes, int outlen, unsigned char lcid, unsigned int *sn, unsigned char *bytes, int len);
int ping(unsigned char * bytes, int outlen, unsigned int source_ip, unsigned int dest_ip);
int rrc(sdu *sdus, int outlen, unsigned char lcid, unsigned char * bytes, int len);
int end_to_end(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen);
void reset_state();
unsigned short checksum(unsigned char * bytes, unsigned int len);
int ip(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen);
int icmp(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen);
int udp(unsigned char * rx_bytes, int rx_len, unsigned char * tx_bytes, int outlen);
