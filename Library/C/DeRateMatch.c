//#include <bb/complex.h>
#include "complex.h"
#include <string.h>
#include "DeRateMatch.h"
#include "DeRateMatch.txt"

#define BlockLen 5504
#define TurboOutLen 16524
#define RateMatchLen 7200
#define DecodeTimes 8

void DeRataMatch(char * input,char * output);

extern short LUT_Internal[5504];

void DeRateMatch(char * DeScrambleOut, char * DeRateMatchOut)
{
	int i;
	memset(DeRateMatchOut,0,176640*sizeof(char));
	//SubFrame1
	for(i=0;i<DecodeTimes;i++)
	{
		DeRataMatch(&DeScrambleOut[RateMatchLen*i],&DeRateMatchOut[22080*i]);
	}
}

__forceinline
void DeRataMatch(char * input,char * output)
{
	int i;
	for(i=0;i<7200;i++)
	{
		output[LUT_DeRateMatch[i]]=input[i];
	}
	for(i=0;i<BlockLen;i++)
	{
		output[(BlockLen+16)*2+i]=output[LUT_Internal[i]];
	}
	return;	
}