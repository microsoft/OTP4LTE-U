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

#include "mac.h"
#include "jsmn.h"
#include <string.h>
#include <stdlib.h>

const int bufflen = 1024;


const char* kLocalIP = "192.168.1.11";
const long kLocalPort = 4444;
int parseJson(const char* jsonstr);
/*
const char *EXAMPLE =
"{\"user\": \"johndoe\", \"admin\": false, \"uid\": 1000,\n  "
"\"array\": [\"users\", \"wheel\", \"audio\", \"video\"]}";
*/
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING
		&& (int)strlen(s) == tok->end - tok->start
		&& strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

static int inoncycle = 0; // in ms
static int inoffcycle = 0; //in ms
static int ininterval = 0; // in ms
//for CTS
static int ctsduration = 0; //in us
static int ncts = 0;
static int ctsgap = 0; //in us

DWORD WINAPI go_thread_udpsocket(void *pParam)
{
	thread_info *ti = (thread_info *)pParam;
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[bufflen];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(kLocalIP);
	server.sin_port = htons(kLocalPort);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		//exit(EXIT_FAILURE);
		return NULL;
	}
	puts("Bind done");

	int multi_use = 1;
	int check_mu = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&multi_use, sizeof(multi_use));
	if (-1 == check_mu) {
		printf("Set multiple use on same address failed with error code : %d", WSAGetLastError());
		//exit(EXIT_FAILURE);
		return NULL;
	}
	puts("Set Multi USE Done!");
	//keep listening for data
	while (true)
	{
		//printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', bufflen);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, bufflen, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		//printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		//printf("Data: %s\n", buf);
		//lteu_context.numberOfUsers = atoi(buf);
		parseJson(buf);
		EnterCriticalSection(&lteu_context.critSec);
		lteu_context.ttLastUpdate = SoraGetTimeofDay(&(params_tx->measurementInfo.tsinfo));
		//printf("oncycle: %d, offcycle: %d, interval: %d, last update: %ll\n", inoncycle, inoffcycle, ininterval, lteu_context.ttLastUpdate);
		lteu_context.updated = true;
		lteu_context.onCycle = inoncycle;
		lteu_context.offCycle = inoffcycle;
		lteu_context.interval = ininterval;
		LeaveCriticalSection(&lteu_context.critSec);


		//now reply the client with the same data
		/*
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		*/
	}

	closesocket(s);
	WSACleanup();
}



int parseJson(const char* jsonstr) {
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[128]; /* We expect no more than 128 tokens */

	jsmn_init(&p);
	r = jsmn_parse(&p, jsonstr, strlen(jsonstr), t, sizeof(t) / sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return -1;
	}
	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return -1;
	}

	char tmp[1024];
	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(jsonstr, &t[i], "oncycle") == 0) {
			/* We may use strndup() to fetch string value */
			//printf("- aps: %.*s\n", t[i+1].end-t[i+1].start, jsonstr + t[i+1].start);
			int len = t[i + 1].end - t[i + 1].start;
			strncpy(tmp, jsonstr + t[i + 1].start, len);
			tmp[len] = '\0';
			inoncycle = atoi(tmp);			
			i++;
		} else if (jsoneq(jsonstr, &t[i], "offcycle") == 0) {
			/* We may additionally check if the value is either "true" or "false" */
			//printf("- stas: %.*s\n", t[i+1].end-t[i+1].start, jsonstr + t[i+1].start);
			int len = t[i + 1].end - t[i + 1].start;
			strncpy(tmp, jsonstr + t[i + 1].start, len);
			tmp[len] = '\0';
			inoffcycle = atoi(tmp);
			i++;
		} else if (jsoneq(jsonstr, &t[i], "interval") == 0) {
			/* We may want to do strtol() here to get numeric value */
			//printf("- links: %.*s\n", t[i+1].end-t[i+1].start, jsonstr + t[i+1].start);
			int len = t[i + 1].end - t[i + 1].start;
			strncpy(tmp, jsonstr + t[i + 1].start, len);
			tmp[len] = '\0';
			ininterval = atoi(tmp);
			i++;
		} else if (jsoneq(jsonstr, &t[i], "ctsduration") == 0) {
			/* We may want to do strtol() here to get numeric value */
			//printf("- links: %.*s\n", t[i+1].end-t[i+1].start, jsonstr + t[i+1].start);
			int len = t[i + 1].end - t[i + 1].start;
			strncpy(tmp, jsonstr + t[i + 1].start, len);
			tmp[len] = '\0';
			ctsduration = atoi(tmp);
			i++;
		} else if (jsoneq(jsonstr, &t[i], "ncts") == 0) {
			/* We may want to do strtol() here to get numeric value */
			//printf("- links: %.*s\n", t[i+1].end-t[i+1].start, jsonstr + t[i+1].start);
			int len = t[i + 1].end - t[i + 1].start;
			strncpy(tmp, jsonstr + t[i + 1].start, len);
			tmp[len] = '\0';
			ncts = atoi(tmp);
			i++;
		} else if (jsoneq(jsonstr, &t[i], "ctsgap") == 0) {
			/* We may want to do strtol() here to get numeric value */
			//printf("- links: %.*s\n", t[i+1].end-t[i+1].start, jsonstr + t[i+1].start);
			int len = t[i + 1].end - t[i + 1].start;
			strncpy(tmp, jsonstr + t[i + 1].start, len);
			tmp[len] = '\0';
			ctsgap = atoi(tmp);
			i++;
		} else if (jsoneq(jsonstr, &t[i], "array") == 0) {
			int j;
			printf("array:\n");
			if (t[i + 1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i + 1].size; j++) {
				jsmntok_t *g = &t[i + j + 2];
				printf("  * %.*s\n", g->end - g->start, jsonstr + g->start);
			}
			i += t[i + 1].size + 1;
		} else {
			printf("Unexpected key: %.*s\n", t[i].end - t[i].start, jsonstr + t[i].start);
		}
	}
	
	return 0;
}


int gettimeofday(struct timeval *tv)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag = 0;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		tmpres /= 10;  /*convert into microseconds*/
		/*converting file time to unix epoch*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	return 0;
}

