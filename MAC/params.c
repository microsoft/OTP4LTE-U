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
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "wpl_alloc.h"
#include <params.h>
#include "sora_radio.h"
#include "mac.h"


Repetitions parse_repeat(char *rp) 
{
	Repetitions reps;
	if (strcmp(rp,"*") == 0)
		return INF_REPEAT;
	reps = strtol(rp,NULL,10); // TODO: Error handling? 
	return reps;
}



// Initializers for default values
bool parse_on_off(char *typ)
{
	if (strcmp(typ, "on") == 0)		return true;
	if (strcmp(typ, "off") == 0)	return false;
	fprintf(stderr, "Error: cannot parse type parameter %s\n", typ);
	exit(1);
}


// Initializers for default values
BlinkFileType parse_type(char *typ) 
{
	if (strcmp(typ,"file") == 0)	return TY_FILE; 
	if (strcmp(typ,"dummy") == 0)	return TY_DUMMY;
	if (strcmp(typ,"sora") == 0)	return TY_SDR;
	if (strcmp(typ,"ip") == 0)		return TY_IP;
	if (strcmp(typ, "memory") == 0) return TY_MEM;
	fprintf(stderr, "Error: cannot parse type parameter %s\n", typ);
	exit(1);
}


int parse_MAC(char *typ)
{
	if (strcmp(typ, "1-thread") == 0)	return 0;
	if (strcmp(typ, "2-threads") == 0)	return 1;
	fprintf(stderr, "Error: cannot parse MAC parameter %s\n", typ);
	exit(1);
}


BlinkFileMode parse_mode(char *md) {
  if (strcmp(md,"dbg") == 0)
    return MODE_DBG; 
  if (strcmp(md,"bin") == 0) 
    return MODE_BIN;
  fprintf(stderr, "Error: cannot parse mode parameter %s\n",md);
  exit(1);
}

int parse_timeStampAtRead(char *md) {
	if (strcmp(md, "read") == 0)
		return 1;
	if (strcmp(md, "write") == 0)
		return 0;
	fprintf(stderr, "Error: cannot parse mode parameter %s\n", md);
	exit(1);
}

int parse_algorithm(char *alg) {
	if (strcmp(alg, "random") == 0)
		return 0;
	if (strcmp(alg, "cast") == 0)
		return 1;
	if (strcmp(alg, "smart") == 0)
		return 2;
	fprintf(stderr, "Error: cannot parse mode parameter %s\n", alg);
	exit(1);
}

#ifdef SORA_PLATFORM
// TODO: Add error handling
ULONG parse_radioID(char *rp) 
{
	return (ULONG) strtol(rp,NULL,10);
}


// TODO: Add error handling
ULONG parse_Amp(char *rp) 
{
	return (ULONG) strtol(rp,NULL,10);
}


// TODO: Add error handling
ULONG parse_CentralFrequency(char *rp) 
{
	/*bug found, if the rp is 2412M
	fprintf(stdout, "in parse central frequence: %s\n", rp);
	fprintf(stdout, "in parse frequence: %u\n", (ULONG)strtol(rp, NULL, 10));
	fprintf(stdout, "in parse frequence: %u\n", (ULONG)atof(rp));
	return (ULONG) strtol(rp,NULL,10);
	*/
	return (ULONG)atof(rp);
}


// TODO: Add error handling
LONG parse_FrequencyOffset(char *rp) 
{
	return (LONG) strtol(rp,NULL,10);
}


// TODO: Add error handling
ULONG parse_SampleRate(char *rp) 
{
	return (ULONG) strtol(rp,NULL,10);
}


// TODO: Add error handling
ULONG parse_TXBufferSize(char *rp) 
{
	return (ULONG) strtol(rp,NULL,10);
}
#endif



unsigned long parse_size (char *rp) {
  return (strtol(rp,NULL,10));
}


memsize_int parse_mem_size(char *rp) {
	return (strtoll(rp, NULL, 10));
}

int parse_arr(char *str, int* arr) {
	const int MAX_PARSE_ARR_SIZE = 255;
	int len = 0;
	int oldi = 0;
	int i;

	for (i = 0; i < MAX_PARSE_ARR_SIZE && str[i] != 0; i++)
	{
		if (str[i] == ',')
		{
			str[i] = '\0';
			arr[len] = strtol((str + oldi), NULL, 10);
			str[i] = ',';
			len++;
			oldi = i + 1;
		}
	}
	if (i > oldi)
	{
		arr[len] = strtol((str + oldi), NULL, 10);
		len++;
	}
	return len;
}



// Threshold for energy detection tests
extern int32 energyDetectionThreshold;
void init_energyDetThr(BlinkParams *params, char *typ)			{ energyDetectionThreshold = parse_size(typ); }

// Level of debug printout
extern int32 debug_level;
void init_debugLevel(BlinkParams *params, char *typ)			{ debug_level = parse_size(typ); }

// Use local test IP responder
extern bool test_IP;
void init_testIP(BlinkParams *params, char *typ)				{ test_IP = parse_on_off(typ); }

// MAC Unit tests
extern bool runMACTests;
void init_MACtests(BlinkParams *params, char *typ)				{ runMACTests = parse_on_off(typ); }

// LTE Band
extern int LTEBand;
void init_LTEBand(BlinkParams *params, char *typ)				{ LTEBand = parse_size(typ); }

// dnlinkNRB
extern int dnlinkNRB;
void init_dnlinkNRB(BlinkParams *params, char *typ)				{ dnlinkNRB = parse_size(typ); }

// dnlinkNRB
extern int dnlinkMCS;
void init_dnlinkMCS(BlinkParams *params, char *typ)				{ dnlinkMCS = parse_size(typ); }

//emptySubF
extern int noEmptySubF;
extern int emptySubF[10];
void init_emptySubF(BlinkParams *params, char *typ)				{ noEmptySubF = parse_arr(typ, emptySubF); }

//dataSubF
extern int noDataSubF;
extern int dataSubF[10];
void init_dataSubF(BlinkParams *params, char *typ)				{ noDataSubF = parse_arr(typ, dataSubF); }

// SIB_SF
extern int SIB1SF;
extern int SIB2SF;
void init_SIB1SF(BlinkParams *params, char *typ)				{ SIB1SF = parse_size(typ); }
void init_SIB2SF(BlinkParams *params, char *typ)				{ SIB2SF = parse_size(typ); }



void init_cmdFifoQueueSizeTX(BlinkParams *params, char *typ)	{ cmd_fifo_queue_size = (int)parse_size(typ); }
void init_noTxBufsTX(BlinkParams *params, char *typ)			{ no_tx_bufs = (int)parse_size(typ); }


// TX Init functions
void init_inTypeTX(BlinkParams *params, char *typ)				{ params[0].inType = parse_type(typ); }
void init_outTypeTX(BlinkParams *params, char *typ)				{ params[0].outType = parse_type(typ); }
void init_inFileTX(BlinkParams *params, char *fn)				{ params[0].inFileName = fn; }
void init_outFileTX(BlinkParams *params, char *fn)				{ params[0].outFileName = fn; }
void init_inFileModeTX(BlinkParams *params, char *md)			{ params[0].inFileMode = parse_mode(md); }
void init_outFileModeTX(BlinkParams *params, char *md)			{ params[0].outFileMode = parse_mode(md); }
void init_outBufTX(BlinkParams *params, char *siz)				{ params[0].outBufSize = parse_size(siz); }
void init_inMemorySizeTX(BlinkParams *params, char *size)		{ params[0].inMemorySize = parse_size(size); }
void init_outMemorySizeTX(BlinkParams *params, char *size)		{ params[0].outMemorySize = parse_size(size); }
void init_dummySamplesTX(BlinkParams *params, char *siz)		{ params[0].dummySamples = parse_repeat(siz); }
void init_heapSizeTX(BlinkParams *params, char *siz)			{ params[0].heapSize = parse_size(siz); }
void init_inRepeatTX(BlinkParams *params, char *i)				{ params[0].inFileRepeats = parse_repeat(i); }
void init_LatencySamplingTX(BlinkParams *params, char *siz)		{ params[0].latencySampling = parse_size(siz); }
void init_TimeStampAtReadTX(BlinkParams *params, char *typ)		{ params[0].timeStampAtRead = parse_timeStampAtRead(typ); }
void init_LatencyCDFSizeTX(BlinkParams *params, char *siz)		{ params[0].latencyCDFSize = parse_size(siz); }

// LTE-U related parameters
void init_OnCycle(BlinkParams *params, char *siz)				{ lteu_context.onCycle = parse_size(siz); }
void init_OffCycle(BlinkParams *params, char *siz)				{ lteu_context.offCycle = parse_size(siz); }
void init_DutyCycle_Algorithm(BlinkParams *params, char *alg)	{ lteu_context.algorithm = parse_algorithm(alg); }
void init_CTS_Subframes(BlinkParams *params, char *siz)			{ lteu_context.ctssubframes = parse_size(siz); }

#ifdef SORA_PLATFORM
void init_radioIdTX(BlinkParams *params, char *i)				{ params[TARGET_RADIO_TX].radioParams.radioId = parse_radioID(i); }
void init_gainTX(BlinkParams *params, char *i)					{ params[TARGET_RADIO_TX].radioParams.TXgain = parse_Amp(i); }
void init_centralFrequencyTX(BlinkParams *params, char *i)		{ params[TARGET_RADIO_TX].radioParams.CentralFrequency = parse_CentralFrequency(i); }
void init_freqencyOffsetTX(BlinkParams *params, char *i)		{ params[TARGET_RADIO_TX].radioParams.FreqencyOffset = parse_FrequencyOffset(i); }
void init_sampleRateTX(BlinkParams *params, char *i)			{ params[TARGET_RADIO_TX].radioParams.SampleRate = parse_SampleRate(i); }
void init_bandwidthTX(BlinkParams *params, char *i)				{ params[TARGET_RADIO_TX].radioParams.Bandwidth = parse_SampleRate(i); }
void init_bladerfDCBias(BlinkParams *params, char *i)			{ params[TARGET_RADIO_TX].radioParams.DCBias = parse_SampleRate(i); }
void init_bufferSizeTX(BlinkParams *params, char *i)			{ params[TARGET_RADIO_TX].radioParams.TXBufferSize = parse_TXBufferSize(i); }
#endif



// RX Init functions
void init_inTypeRX(BlinkParams *params, char *typ)				{ params[1].inType = parse_type(typ); }
void init_outTypeRX(BlinkParams *params, char *typ)				{ params[1].outType = parse_type(typ); }
void init_inFileRX(BlinkParams *params, char *fn)				{ params[1].inFileName = fn; }
void init_outFileRX(BlinkParams *params, char *fn)				{ params[1].outFileName = fn; }
void init_inFileModeRX(BlinkParams *params, char *md)			{ params[1].inFileMode = parse_mode(md); }
void init_outFileModeRX(BlinkParams *params, char *md)			{ params[1].outFileMode = parse_mode(md); }
void init_outBufRX(BlinkParams *params, char *siz)				{ params[1].outBufSize = parse_size(siz); }
void init_inMemorySizeRX(BlinkParams *params, char *size)		{ params[1].inMemorySize = parse_size(size); }
void init_outMemorySizeRX(BlinkParams *params, char *size)		{ params[1].outMemorySize = parse_size(size); }
void init_dummySamplesRX(BlinkParams *params, char *siz)		{ params[1].dummySamples = parse_repeat(siz); }
void init_heapSizeRX(BlinkParams *params, char *siz)			{ params[1].heapSize = parse_size(siz); }
void init_inRepeatRX(BlinkParams *params, char *i)				{ params[1].inFileRepeats = parse_repeat(i); }
void init_LatencySamplingRX(BlinkParams *params, char *siz)		{ params[1].latencySampling = parse_size(siz); }
void init_TimeStampAtReadRX(BlinkParams *params, char *typ)		{ params[1].timeStampAtRead = parse_timeStampAtRead(typ); }
void init_LatencyCDFSizeRX(BlinkParams *params, char *siz)		{ params[1].latencyCDFSize = parse_size(siz); }


#ifdef SORA_PLATFORM
// Radio IDs are 
void init_radioIdRX(BlinkParams *params, char *i)				{ params[TARGET_RADIO_RX].radioParams.radioId = parse_radioID(i); }
void init_paRX(BlinkParams* params, char *i)					{ params[TARGET_RADIO_RX].radioParams.RXpa = parse_Amp(i); }
void init_gainRX(BlinkParams* params, char *i)					{ params[TARGET_RADIO_RX].radioParams.RXgain = parse_Amp(i); }
void init_centralFrequencyRX(BlinkParams *params, char *i)		{ params[TARGET_RADIO_RX].radioParams.CentralFrequency = parse_CentralFrequency(i); }
void init_freqencyOffsetRX(BlinkParams *params, char *i)		{ params[TARGET_RADIO_RX].radioParams.FreqencyOffset = parse_FrequencyOffset(i); }
void init_sampleRateRX(BlinkParams *params, char *i)			{ params[TARGET_RADIO_RX].radioParams.SampleRate = parse_SampleRate(i); params[TARGET_RADIO_RX].radioParams.Bandwidth = params->radioParams.SampleRate; }
#endif



// Here is where we declare the parameters
#ifdef SORA_PLATFORM
#define PARAM_TABLE_LENGTH		59
#else
#define PARAM_TABLE_LENGTH		46
#endif

BlinkParamInfo paramTable[PARAM_TABLE_LENGTH] = 
// TX
  {  { "--memory-size=",
	   "--memory-size=<bytes>",
       "Size of memory buffer between TX and RX blocks",
       "0",
	   init_outMemorySizeTX },
	 { "--energy-detection-threshold=",
	   "--energy-detection-threshold=<threshold>",
	   "Threshold for energy detection debug procedure. Set to 0 to switch off energy detection (faster code)",
	   "0",
	   init_energyDetThr },
	   { "--debug-level=",
	   "--debug-level=<level>",
	   "Set which debug messages to be printed. Check mac.h for details.",
	   "1024",
	   init_debugLevel },
	   { "--test-IP=",
	   "--test-IP=on|off",
	   "Don't pass IP packets to NDIS but process them locally by only responding to pings (for test).",
	   "off",
	   init_testIP },
	   { "--MAC-unit-tests=",
	   "--MAC-unit-tests=on|off",
	   "Set to <on> to run unit tests on MAC",
	   "off",
	   init_MACtests },
	   { "--LTE-band=",
	   "--LTE-band=<number>",
	   "3GPP LTE band",
	   "17",
	   init_LTEBand },
	   { "--Downlink-NRB=",
	   "--Downlink-NRB=<number>",
	   "Number of resource blocks to be used for downlink transmissions of IP packets (default -1 means either use MCS or hardcoded default)",
	   "-1",
	   init_dnlinkNRB },
	   { "--Downlink-MCS=",
	   "--Downlink-MCS=<number>",
	   "MCS to be used for downlink transmissions of IP packets (default -1 means either use downlink_NRB or hardcoded default)",
	   "-1",
	   init_dnlinkMCS },
	   { "--Empty-SubF=",
	   "--Empty-SubF=<number>,<number>,...",
	   "Array of subframes to be left completely empty (no RS), simulating LTE-U off cycle",
	   "",
	   init_emptySubF },
	   { "--Data-SubF=",
	   "--Data-SubF=<number>,<number>,...",
	   "Array of subframes to be used for data/IP transmissions",
	   "4",
	   init_dataSubF },
	   { "--SIB1SF=",
	   "--SIB1SF=<number>",
	   "Index of subframe that carries SIB1",
	   "5",
	   init_SIB1SF },
	   { "--SIB2SF=",
	   "--SIB2SF=<number>",
	   "Index of subframe that carries SIB2",
	   "0",
	   init_SIB2SF },
	   { "--TX-cmd-fifo-queue-size=",
	   "--TX-cmd-fifo-queue-size=<number>",
	   "Number of buffers (of size TX-sora-tx-buffer-size samples) in firmware's queue",
	   "2",
	   init_cmdFifoQueueSizeTX },
	 { "--TX-no-tx-bufs=",
	   "--TX-no-tx-bufs=<number>",
	   "Number of buffers (of size TX-sora-tx-buffer-size samples) in Ziria's queue",
	   "2",
	   init_noTxBufsTX },
	 { "--TX-input=",
       "--TX-input=file|dummy|sora|ip",
       "Input TX samples come from a file, radio, or are dummy samples",
       "file",
       init_inTypeTX },
     { "--TX-output=", 
       "--TX-output=memory|file|dummy|sora|ip",
       "Output TX samples written to file, radio, or are ignored (dummy)",
       "file",
       init_outTypeTX },
     { "--TX-input-file-name=", 
       "--TX-input-file-name=...",
       "Path to TX input file. Meaningful if --TX-input=file.",
       "infile", 
       init_inFileTX },
     { "--TX-output-file-name=", 
       "--TX-output-file-name=...",
       "Path to TX output file. Meaningful if --TX-output=file.",
       "outfile", 
       init_outFileTX },
     { "--TX-input-file-mode=", 
       "--TX-input-file-mode=bin|dbg",
       "TX Input file is binary/debug (human readable). Meaningful if --TX-input=file.",
       "bin",
       init_inFileModeTX },
     { "--TX-output-file-mode=", 
       "--TX-output-file-mode=bin|dbg",
       "Output file is binary/debug (human readable). Meaningful if --TX-output=file.",
       "bin",
       init_outFileModeTX },
     { "--TX-output-buffer-size=", 
       "--TX-output-buffer-size=...",
       "TX Output buffer out of which we write to the output.",
       "0",					
       init_outBufTX },
     { "--TX-dummy-samples=", 
       "--TX-dummy-samples==*|1|2|...",
       "Number of dummy TX samples (* means repeated indefinitely). Meaningful if --TX-input=dummy",
       "20000000",
       init_dummySamplesTX },
     { "--TX-heap-size=", 
       "--TX-heap-size=...",
       "Size of heap to use in the generated TX program.",
       "20971520",     // 20MB default
       init_heapSizeTX },
     { "--TX-input-file-repeat=",
       "--TX-input-file-repeat=*|1|2|....",
       "Repeat the TX input file indefinitely (*) or a fixed number of times",
       "1", 
       init_inRepeatTX },
     { "--TX-latency-sampling=",
	   "--TX-latency-sampling=0|1|2|...",
	   "Number of writes over which the TX latency is measured (0 - no latency measurements)",
	   "0",
	   init_LatencySamplingTX },
	 { "--TX-latency-sampling-location=",
	   "--TX-latency-sampling-location=read|write",
	   "Where to sample intra-sample latency, at read or write",
	   "read",
	   init_TimeStampAtReadTX },
	 { "--TX-latency-CDF-size=",
	   "--TX-latency-CDF-size=...",
	   "Number of TX latency samples to be stored and printed (for CDF calculation)",
	   "0",
	   init_LatencyCDFSizeTX },

	   { "--TX-oncycle=",
	   "--TX-oncycle=...",
	   "Number of ms LTE-U is on",
	   "100",
	   init_OnCycle },
	   { "--TX-offcycle=",
	   "--TX-offcycle=...",
	   "Number of ms LTE-U is off",
	   "0",
	   init_OffCycle },
	   { "--TX-dutycycle-algorithm=",
	   "--TX-dutycycle=cast|random|smart",
	   "Algorithm used to adjust duty cycle",
	   "random",
	   init_DutyCycle_Algorithm },
	   { "--TX-cts-subframes=",
	   "--TX-ctssubframes=0,1,2",
	   "number of subframes used to send cts",
	   "0",
	   init_CTS_Subframes },
//RX
	   { "--RX-input=",
	   "--RX-input=memory|file|dummy|sora|ip",
	   "Input RX samples come from a file, radio, or are dummy samples",
	   "file",
	   init_inTypeRX },
	   { "--RX-output=",
	   "--RX-output=file|dummy|sora|ip",
	   "Output RX samples written to file, radio, or are ignored (dummy)",
	   "file",
	   init_outTypeRX },
	   { "--RX-input-file-name=",
	   "--RX-input-file-name=...",
	   "Path to RX input file. Meaningful if --RX-input=file.",
	   "infile",
	   init_inFileRX },
	   { "--RX-output-file-name=",
	   "--RX-output-file-name=...",
	   "Path to RX output file. Meaningful if --RX-output=file.",
	   "outfile",
	   init_outFileRX },
	   { "--RX-input-file-mode=",
	   "--RX-input-file-mode=bin|dbg",
	   "RX Input file is binary/debug (human readable). Meaningful if --RX-input=file.",
	   "bin",
	   init_inFileModeRX },
	   { "--RX-output-file-mode=",
	   "--RX-output-file-mode=bin|dbg",
	   "Output file is binary/debug (human readable). Meaningful if --RX-output=file.",
	   "bin",
	   init_outFileModeRX },
	   { "--RX-output-buffer-size=",
	   "--RX-output-buffer-size=...",
	   "RX Output buffer out of which we write to the output.",
	   "16777216",
	   init_outBufRX },
	   { "--RX-dummy-samples=",
	   "--RX-dummy-samples==*|1|2|...",
	   "Number of dummy RX samples (* means repeated indefinitely). Meaningful if --RX-input=dummy",
	   "20000000",
	   init_dummySamplesRX },
	   { "--RX-heap-size=",
	   "--RX-heap-size=...",
	   "Size of heap to use in the generated RX program.",
	   "20971520",     // 20MB default
	   init_heapSizeRX },
	   { "--RX-input-file-repeat=",
	   "--RX-input-file-repeat=*|1|2|....",
	   "Repeat the RX input file indefinitely (*) or a fixed number of times",
	   "1",
	   init_inRepeatRX },
	   { "--RX-latency-sampling=",
	   "--RX-latency-sampling=0|1|2|...",
	   "Number of writes over which the RX latency is measured (0 - no latency measurements)",
	   "0",
	   init_LatencySamplingRX },
	   { "--RX-latency-sampling-location=",
	   "--RX-latency-sampling-location=read|write",
	   "Where to sample intra-sample latency, at read or write",
	   "read",
	   init_TimeStampAtReadRX },
	   { "--RX-latency-CDF-size=",
	   "--RX-latency-CDF-size=...",
	   "Number of RX latency samples to be stored and printed (for CDF calculation)",
	   "0",
	   init_LatencyCDFSizeRX },


#ifdef SORA_PLATFORM
       { "--TX-sora-radio-id=",
       "--TX-sora-radio-id = ...",
       "Sora TX radio ID",
       "0", 
       init_radioIdTX }
     , { "--TX-sora-gain=",
       "--TX-sora-gain = ...",
       "Sora TX gain",
       "0", 
       init_gainTX }
     , { "--TX-sora-central-frequency=",
       "--TX-sora-central-frequency = ...",
       "Sora central frequency in MHz (default 578)",
       "578", 
       init_centralFrequencyTX }
     , { "--TX-sora-freqency-offset=",
       "--TX-sora-freqency-offset = ...",
       "Sora frequency offset",
       "0", 
       init_freqencyOffsetTX }
     , { "--TX-sora-sample-rate=",
       "--TX-sora-sample-rate = ...",
       "Sora/BaldeRF sample rate",
       "15360000", 
       init_sampleRateTX }
	   , { "--TX-sora-bandwidth=",
	   "--TX-sora-bandwidth = ...",
	   "Sora/BladeRF bandwdith",
	   "15360000",
	   init_bandwidthTX }
	   , { "--TX-bladerf-dcbias=",
	   "--TX-bladerf-dcbias = ...",
	   "BladeRF dc bias",
	   "10",
	   init_bladerfDCBias }
	   , { "--TX-sora-tx-buffer-size=",
       "--TX-sora-tx-buffer-size = ...",
       "Size of the TX buffer transmitted at once (in number of complex16)",
       "131072",					// 4*16*2048
       init_bufferSizeTX }
// RX
	 , { "--RX-sora-radio-id=",
		"--RX-sora-radio-id = ...",
		"Sora RX radio ID",
		"0",
		init_radioIdRX }
	 , { "--RX-sora-rx-pa=",
		 "--RX-sora-rx-pa = ...",
		 "Sora RX power amplification",
		 "0",
		 init_paRX }
	 , { "--RX-sora-rx-gain=",
		 "--RX-sora-rx-gain = ...",
		 "Sora RX gain",
		 "0",
		 init_gainRX }
	 , { "--RX-sora-gain=",
		 "--RX-sora-gain = ...",
		 "Sora RX gain",
		 "0",
		 init_gainRX }
	 , { "--RX-sora-central-frequency=",
		 "--RX-sora-central-frequency = ...",
		 "Sora RX central frequency in MHz (default 578)",
		 "578",
		 init_centralFrequencyRX }
	 , { "--RX-sora-freqency-offset=",
		 "--RX-sora-freqency-offset = ...",
		 "Sora RX frequency offset",
		 "0",
		 init_freqencyOffsetRX }
	 , { "--RX-sora-sample-rate=",
		 "--RX-sora-sample-rate = ...",
		 "Sora RX sample rate",
		 "40",
		 init_sampleRateRX }

#endif


};


void print_blink_usage() {
  int i = 0;
  fprintf(stdout,"Usage: <cmd> OPTS...\n");
  fprintf(stdout,"OPTS\n");
  while (i < sizeof(paramTable)/sizeof(BlinkParamInfo)) {
    fprintf(stdout,"%s\n", paramTable[i].param_use);
    fprintf(stdout,"\t%s\n", paramTable[i].param_descr);
    fprintf(stdout,"\tDefault: %s\n", paramTable[i].param_dflt);
    i++; 
  }
}


void try_parse_args(BlinkParams *params, int argc, char ** argv) {
  int pi; 
  if (argc <= 1) {
    print_blink_usage();
    exit(1); 
  }

  // Initialize everything to default
  for (pi=0; pi < sizeof(paramTable) / sizeof(BlinkParamInfo); pi++) {
      paramTable[pi].param_init(params, paramTable[pi].param_dflt);
  }

  // For every command line parameter, try to initialize it or fail if unknown
  for (int ai=1; ai < argc; ai++) {
    int initialized = 0;
    for (pi=0; pi < sizeof(paramTable) / sizeof(BlinkParamInfo); pi++) {
	  char *pval;
      if (!strstr(argv[ai],paramTable[pi].param_str)) continue;
      pval = & argv[ai][strlen(paramTable[pi].param_str)];
      paramTable[pi].param_init(params, pval);
      initialized = 1;
      break;
    }
    if (!initialized)
	{
	  fprintf(stderr, "Unknown parameter: %s\n", argv[ai]);
	  print_blink_usage();
	  exit(1);
	}

  }


}
