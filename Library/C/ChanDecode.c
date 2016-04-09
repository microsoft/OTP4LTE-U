#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <bb/complex.h>
//#include "complex.h"
#include <vector128.h>
//#include <bb/basic.h>
#include "ChanDecode.h"
#include "ChanDecode.txt"
#include <emmintrin.h>

#include <types.h>

#ifdef WIN32
#define A16 __declspec(align(16))

template<typename T>
DSP_INLINE static const T __0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF()
{
	const static vub::data_type value =
	{
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF
	};
	return (T)vub(value);
}


#define MAX_BLOCK_LEN	6144




//#define BlockLen 5504
//#define AlignedBlockLen 5520
#define TurboOutLen 16524
#define RateMatchLen 7200
#define DecodeTimes 8
#define IterTimes 1
#define MAX -128


// OLD
/*
A16 char CharOne[16]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
A16 char AlphaBeta[(BlockLen+6)*16];//alpha:1-5508;beta:3-5510
A16 char gamma[(BlockLen+16)*4];
A16 char NePo[(BlockLen+4)*16];//ne:1:5508*8,po:5508*8+1:5508*8+5508*8
A16 char tntp[(BlockLen+3+13)*2];//tn:1:5507,tp:5521:11027

A16 char ext21[BlockLen+3+13];//extrinsic LLR array from dec2 to dec1
A16 char ext12[BlockLen+3+13];//extrinsic LLR array from dec1 to dec2
A16 char ext1[BlockLen+3+13]; //extrinsic llR array to dec1 after deinterleaver
A16 char ext2[BlockLen+3+13]; //extrinsic llr array to dec2 after interleaver; 
*/


A16 char CharOne[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
A16 char AlphaBeta[(MAX_BLOCK_LEN + 6) * 16];//alpha:1-5508;beta:3-5510
A16 char gamma[(MAX_BLOCK_LEN + 16) * 4];
A16 char NePo[(MAX_BLOCK_LEN + 4) * 16];//ne:1:5508*8,po:5508*8+1:5508*8+5508*8
A16 char tntp[(MAX_BLOCK_LEN + 3 + 13) * 2];//tn:1:5507,tp:5521:11027

A16 char ext21[MAX_BLOCK_LEN + 3 + 13];//extrinsic LLR array from dec2 to dec1
A16 char ext12[MAX_BLOCK_LEN + 3 + 13];//extrinsic LLR array from dec1 to dec2
A16 char ext1[MAX_BLOCK_LEN + 3 + 13]; //extrinsic llR array to dec1 after deinterleaver
A16 char ext2[MAX_BLOCK_LEN + 3 + 13]; //extrinsic llr array to dec2 after interleaver; 



A16 char AlphaPara[112]={0,2,4,6,0,2,4,6,1,3,5,7,1,3,5,7,//alpha-l
						8,10,12,14,8,10,12,14,9,11,13,15,9,11,13,15,//alpha-h
						0,2,1,3,3,1,2,0,3,1,2,0,0,2,1,3,//gamma-0
						4,6,5,7,7,5,6,4,7,5,6,4,4,6,5,7,//gamma-1
						8,10,9,11,11,9,10,8,11,9,10,8,8,10,9,11,//gamma-2
						12,14,13,15,15,13,14,12,15,13,14,12,12,14,13,15,//gamma-3
						0,-64,-64,-64,-64,-64,-64,-64,0,0,0,0,0,0,0,0};

A16 char  BetaPara[112]={0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,//beta_l
						 8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,//beta_h
						 8,11,10,9,9,10,11,8,11,8,9,10,10,9,8,11,	//gamma-3
						 4,7,6,5,5,6,7,4,7,4,5,6,6,5,4,7,	//gamma-2
						 0,3,2,1,1,2,3,0,3,0,1,2,2,1,0,3,	//gamma-1
						 12,15,14,13,13,14,15,12,15,12,13,14,14,13,12,15,	//gamma-4
						 0,-64,-64,-64,-64,-64,-64,-64,0,0,0,0,0,0,0,0};

A16 char NePoPara[96]={0,4,5,1,2,6,7,3,8,12,13,9,10,14,15,11,//ne-betaSet
					 4,0,1,5,6,2,3,7,12,8,9,13,14,10,11,15,//po-betaSet
				     0,0,1,1,1,1,0,0,4,4,5,5,5,5,4,4,//ne-gamma,L
					 8,8,9,9,9,9,8,8,12,12,13,13,13,13,12,12,//ne-gamma,H
					 3,3,2,2,2,2,3,3,7,7,6,6,6,6,7,7,//po-gamma,L
					 11,11,10,10,10,10,11,11,15,15,14,14,14,14,15,15};//po-gamma,H

A16 char tntpPara[16]={0,4,2,6,8,12,10,14,1,5,3,7,9,13,11,15};




void __ext_deturbo(unsigned char * output, int __unused_1, int16 BlockLen,
	int8 * input1, int __unused_2, int8 * input2, int __unused_3, int8 * input3, int __unused_4)
{
	DeTurbo(BlockLen, (char *)input1, (char *)input2, (char *)input3, (unsigned char *)output);
}


// Uncomment this to test one-round BCJR algorithm
//#define ONE_STAGE


__forceinline
void DeTurbo(int BlockLen, char * input1, char * input2, char * input3, unsigned char * infobits)
{
	int i,j;
	char intlvd[MAX_BLOCK_LEN];
	int DataLen = BlockLen - 4;		// There are 4 trail bits

	memset(&ext1, 0, (MAX_BLOCK_LEN + 3) * sizeof(char));
	memset(&ext2, 0, (MAX_BLOCK_LEN + 3) * sizeof(char));

	// Not sure why this needs to be reset, but it seems it needs to be. 
	// tntp is only being written AlignedBlockLen / 16-1 times in TpS and TnS
	// but it is being read in AlignedBlockLen / 16 times in ExOS
	// So it seems to be implied that unwritten bits are set to zero!
	memset(tntp, 0, ((MAX_BLOCK_LEN + 3 + 13) * 2) * sizeof(char));


	// *** Preparation: SYS[0..5519], PAR1[00..5519], Interl(SYS)[0..5519], PAR2[0..5519]
	
	/* Old interleaver, hardcoded for 5504 block
	for (i = 0; i<BlockLen; i++)
	{
		intlvd[i] = input1[LUT_Internal[i]];
	}
	*/
	interleave(intlvd, input1, DataLen, false);


    for(i = 0; i < IterTimes; i++)
	{
		LogMap(BlockLen, input1, input2, &ext1[0], &ext12[0]);//1st LogMap
		/* Old interleaver, hardcoded for 5504 block
		for(j = 0; j < BlockLen; j++)
		{
			ext2[j] = ext12[LUT_Internal[j]];
		}
		*/
		interleave(ext2, ext12, DataLen, false);

#ifndef ONE_STAGE
		LogMap(BlockLen, intlvd, input2, &ext2[0], &ext21[0]);//2nd LogMap 
		/* Old interleaver, hardcoded for 5504 block
		for(j = 0; j < BlockLen; j++)
		{
			ext1[j] = ext21[LUT_DeInternal[j]];
		}
		*/
		interleave(ext21, ext1, DataLen, true);
#endif
	}

	/*
	// DEBUG LLR output
	for (i = 0; i < DataLen; i++)
	{
		//infobits[i] = (LLR[i] + ext1[i] + ext12[i]);
		infobits[i] = (input1[i] + ext1[i] + ext12[i]) > 0;
	}
	*/

	//Output
	for(i=0;i<BlockLen/8;i++)
	{
		//1st bit
		infobits[i] = (input1[8*i]+ext1[8*i]+ext12[8*i]) > 0;
		
		//2rd bit
		infobits[i] = infobits[i] + 2*((input1[8*i+1] + ext1[8*i+1] + ext12[8*i+1]) > 0);

		//3th bit
		infobits[i] = infobits[i] + 4*((input1[8*i+2] + ext1[8*i+2] + ext12[8*i+2]) > 0);

		//4th bit
		infobits[i] = infobits[i] + 8*((input1[8*i+3] + ext1[8*i+3] + ext12[8*i+3]) > 0);

		//5th bit
		infobits[i] = infobits[i] + 16*((input1[8*i+4] + ext1[8*i+4] + ext12[8*i+4]) > 0);

		//6th bit
		infobits[i] = infobits[i] + 32*((input1[8*i+5] + ext1[8*i+5] + ext12[8*i+5]) > 0);

		//7th bit
		infobits[i] = infobits[i] + 64*((input1[8*i+6] + ext1[8*i+6] + ext12[8*i+6]) > 0);

		//8th bit
		infobits[i] = infobits[i] + 128*((input1[8*i+7] + ext1[8*i+7] + ext12[8*i+7]) > 0);
	}
	return;
}


__forceinline void LogMap(int BlockLen, char * Sys, char* Parity, char * extI, char * extO)
{
	char* _gamma = &gamma[0];
	char* _AlphaBeta = &AlphaBeta[0];
	char* _NePo = &NePo[0];
	char* _tntp = &tntp[0];
	
	GammaS(BlockLen, Sys, Parity, extI, _gamma);
	AlphaS(BlockLen, _AlphaBeta, _gamma);
	// now Beta -
	BetaS(BlockLen, _AlphaBeta, _gamma);
	// NePo
	NePoS(BlockLen, _AlphaBeta, _gamma, _NePo);
	//tntp
	TnS(BlockLen, _NePo, _tntp);
	TpS(BlockLen, _NePo, _tntp);
	ExOS(BlockLen, _tntp, extI, extO);

	return;
}


// Inputs:
// Sys - systematic bits, as bytes in range -7:7 (-7 -> 0, 7 -> 1)
// Parity - parity/check bits, as bytes in range -7:7 (-7 -> 0, 7 -> 1)
// extI - prior
// alignedLength - length of input sequences, aligned to 16-bit SSE representation
// (e.g. for actual length 5508 alignedLength is 5520 as the first larger int multiple of 16)
// 
// Output:
// _gamma - BCJR gamma coefficients
void GammaS(int BlockLen, char* Sys, char* Parity, char* extI, char* _gamma)
{
		int AlignedBlockLen = (BlockLen % 16 == 0) ? BlockLen : ((BlockLen / 16 + 1) * 16);
		vcs COne;
		vcs xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
		memcpy(&COne,CharOne,sizeof(vcs));
		vcs* pSys = (vcs*)Sys;
		vcs* pParity = (vcs*)Parity;
		vcs* pextI = (vcs*)extI;
        vcs* pGamma = (vcs*)_gamma;
		int count = AlignedBlockLen / 16;

        xmm7 = __0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF<vcs>();
        xmm6 = COne;

	do{   

		// xmm3 - Systematic
        xmm3 = *pSys;
		// xmm1 - External
		xmm1 = permutate<0xe4>(*pextI);// use shuf to get the second
		// xmm4 - code
		xmm4 = permutate<0xe4>(*pParity);// load code

        xmm3 = saturated_add((vb)xmm3,(vb)xmm1);// sys(15:0)+exti(15:0)
        *pextI = xmm3; // temp save, future use
		
		xmm2 = xmm3; // duplicate
        xmm3 = saturated_add((vb)xmm3,(vb)xmm4);// sys + exti + code : g4
        xmm2 = saturated_sub((vb)xmm2,(vb)xmm4);// sys + exti - code : g3
		
        // compute g1 and g2
        xmm0 = xmm3;
        xmm1 = xmm2;
        xmm0 = xor(xmm0,xmm7);
        xmm1 = xor(xmm1,xmm7);
        xmm0 = add((vb)xmm0,(vb)xmm6);// - sys - exti - code : g1
        xmm1 = add((vb)xmm1,(vb)xmm6);// - sys - exti + code : g2

		// re-arrangement: xmm3, xmm5, xmm2, xmm0
        //xmm0=p0,o0,n0,m0,l0,k0,j0,i0,h0,g0,f0,e0,d0,c0,b0,a0
        //xmm1=p1,o1,n1,m1,l1,k1,j1,i1,h1,g1,f1,e1,d1,c1,b1,a1
        //xmm2=p2,o2,n2,m2,l2,k2,j2,i2,h2,g2,f2,e2,d2,c2,b2,a2
        //xmm3=p3,o3,n3,m3,l3,k3,j3,i3,h3,g3,f3,e3,d3,c3,b3,a3
     
        xmm4 = permutate<0xe4>(xmm0);
        xmm0 = interleave_low((vb)xmm0,(vb)xmm1);
        xmm4 = interleave_high((vb)xmm4,(vb)xmm1);

		xmm5 = permutate<0xe4>(xmm2);
        xmm2 = interleave_low((vb)xmm2,(vb)xmm3);
        xmm5 = interleave_high((vb)xmm5,(vb)xmm3);

	    xmm1 = permutate<0xe4>(xmm0);
        xmm0 = interleave_low((vs)xmm0,(vs)xmm2);
        xmm1 = interleave_high((vs)xmm1,(vs)xmm2);

        xmm3 = permutate<0xe4>(xmm4);
        xmm3 = interleave_low((vs)xmm3,(vs)xmm5);
        xmm4 = interleave_high((vs)xmm4,(vs)xmm5);

	     // save results
        
		//xmm0=p0,p1,p2,p3,o0,o1,o2,o3,n0,n1,n2,n3,m0,m1,m2,m3
		//xmm1=l0,l1,l2,l3,k0,k1,k2,k3,j0,j1,j2,j3,i0,i1,i2,i3
		//xmm3=h0,h1,h2,h3,g0,g1,g2,g3,f0,f1,f2,f3,e0,e1,e2,e3
		//xmm4=d0,d1,d2,d3,c0,c1,c2,c3,b0,b1,b2,b3,a0,a1,a2,a3

        *pGamma = xmm0;
        *(pGamma+1) = xmm1;
        *(pGamma+2) = xmm3;
        *(pGamma+3) = xmm4;

        // advance pointers

        pSys += 1;
		pParity += 1;
        pextI += 1;
        pGamma += 4;
	}while(--count>0);
}

void AlphaS(int BlockLen, char* _AlphaBeta, char* _gamma)
{
	vcs _xmm0, _xmm1, _xmm2, _xmm3, _xmm4, _xmm5, _xmm6, _xmm7;

	vcs* pAlphaBeta = (vcs*)_AlphaBeta;
	vcs* pGamma = (vcs*)_gamma;
	vcs* pAlphaPara = (vcs*)AlphaPara;
	//int count = 1377; // 1377 = (5508*4) (gamma_size) / 16 (aligned)
	int count = BlockLen / 4; // (BlockLen*4) (gamma_size) / 16 (aligned)

	_xmm0 = *(pAlphaPara+6);// init alpha
	_xmm5 = xor(_xmm5,_xmm5);// all 0
	
	do{
		// xmm0 - a(i-1)
		// xmm6 - gamma(i-1)

		_xmm6 = *pGamma;// gamma : load 4 gamma vector at once

		//r0+a0=a1

		_xmm2 = _xmm0;// dup
		// xmm2 all alpha h and l
		__asm
	   {
		   movdqa xmm2, _xmm2;
		   pshufb xmm2, AlphaPara;
		   movdqa _xmm2, xmm2;
		}

		_xmm1 = _xmm6;
		// gamma all
		 __asm
		{
			movdqa xmm1, _xmm1;
			pshufb xmm1, AlphaPara+32;
			movdqa _xmm1, xmm1;
		 }
		// hdd - alpha 1 t1; ldd - alpha 1 t2
		_xmm2 = saturated_add((vb)_xmm2,(vb)_xmm1);
		// ldd - alpha 1 t1; hdd - alpha 1 t2
		_xmm3 = permutate<0x4e>(_xmm2);

		// compute the max
		_xmm2 = smax((vb)_xmm2,(vb)_xmm3);

		// xmm2 - alpha 1
		// normalize
		_xmm4 = _xmm2;
		// spread x0 to all xmm4
		__asm
	   {
		   movdqa xmm4, _xmm4;
		   movdqa xmm5, _xmm5;
		   pshufb xmm4, xmm5  ;
		   movdqa _xmm4, xmm4;
		   movdqa _xmm5, xmm5;
	   }

		// normalize
		_xmm2 = saturated_sub((vb)_xmm2,(vb)_xmm4);
		// alpha 0 and 1 (???)
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm2);

		// store
		// a0, a1 stored
		*pAlphaBeta = _xmm0;

		//r1+a1=a2
		// alpha 1
		__asm
	   {
		   movdqa xmm0, _xmm0;
		   pshufb xmm0, AlphaPara+16;
		   movdqa _xmm0, xmm0;
		}

		// dup gamma
		_xmm1 = _xmm6;
		// gamma all
		__asm
	   {
		   movdqa xmm1, _xmm1;
		   pshufb xmm1, AlphaPara+48;
		   movdqa _xmm1, xmm1;
		}

		// hdd - alpha 2 t1
		_xmm0 = saturated_add((vb)_xmm0,(vb)_xmm1);
		_xmm3 = permutate<0x4e>(_xmm0);

		_xmm0 = smax((vb)_xmm0,(vb)_xmm3);

		// normalize
		_xmm4 = _xmm0;
		__asm
	   {
		   movdqa xmm4, _xmm4;
		   movdqa xmm5, _xmm5;
		   pshufb xmm4, xmm5  ;
		   movdqa _xmm4, xmm4;
		   movdqa _xmm5, xmm5;
	   }

		// xmm0 - ldd - alpha 2
		_xmm0 = saturated_sub((vb)_xmm0,(vb)_xmm4);

		//r2+a2=a3
		//movdqa xmm2, xmm0 ;    // dup
		_xmm2 = _xmm0;
//		pshufb xmm2, [pAlphaPara] ;
		__asm
	   {
		   movdqa xmm2, _xmm2;
		   pshufb xmm2, AlphaPara;
		   movdqa _xmm2, xmm2;
		}
	   
		_xmm1 = _xmm6;
		// gamm all
		__asm
	   {
		   movdqa xmm1, _xmm1;
		   pshufb xmm1, AlphaPara+64;
		   movdqa _xmm1, xmm1;
		}

		//mb,mt;
		_xmm2 = saturated_add((vb)_xmm2,(vb)_xmm1);
		//mt,mb
		_xmm3 = permutate<0x4e>(_xmm2);
		_xmm2 = smax((vb)_xmm2,(vb)_xmm3);
		_xmm4 = _xmm2;
//		pshufb xmm4, xmm5 ;
		
		__asm
	   {
		   movdqa xmm4, _xmm4;
		   movdqa xmm5, _xmm5;
		   pshufb xmm4, xmm5  ;
		   movdqa _xmm4, xmm4;
		   movdqa _xmm5, xmm5;
	   }
		
	   _xmm2 = saturated_sub((vb)_xmm2,(vb)_xmm4);
		// xmm0 - a2, a3
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm2);
		// store 
		*(pAlphaBeta+1) = _xmm0;

		//r3+a3=a4
//		pshufb xmm0, [pAlphaPara+16] ;
		__asm
	   {
		   movdqa xmm0, _xmm0;
		   pshufb xmm0, AlphaPara+16;
		   movdqa _xmm0, xmm0;
		}

		//pshufb xmm6, [pAlphaPara+80] ;
		__asm
	   {
		   movdqa xmm6, _xmm6;
		   pshufb xmm6, AlphaPara+80;
		   movdqa _xmm6, xmm6;
		}

		//mb,mt;
		_xmm0 = saturated_add((vb)_xmm0,(vb)_xmm6);
		//mt,mb
		_xmm3 = permutate<0x4e>(_xmm0);
		_xmm0 = smax((vb)_xmm0,(vb)_xmm3);
		_xmm4 = _xmm0;
//		pshufb xmm4, xmm5 ;
		__asm
	   {
		   movdqa xmm4, _xmm4;
		   movdqa xmm5, _xmm5;
		   pshufb xmm4, xmm5  ;
		   movdqa _xmm4, xmm4;
		   movdqa _xmm5, xmm5;
	   }

		_xmm0 = saturated_sub((vb)_xmm0,(vb)_xmm4);

		pAlphaBeta += 2;
		pGamma += 1;

	}while(--count>0);
}

void BetaS(int BlockLen, char* _AlphaBeta, char* _gamma)
{
	
	// compute alpha
	// p_AlphaBeta : alphabeta (output)
	// p_Gamma : gamma source
	// edx : shuffle parameters
	// count : counter
	vcs _xmm0, _xmm1, _xmm2, _xmm3, _xmm4, _xmm5, _xmm6;
	
	vcs* p_AlphaBeta = (vcs*)_AlphaBeta;
	vcs* pBetaPara = (vcs*)BetaPara;

	//p_AlphaBeta += 5509;
	p_AlphaBeta += BlockLen + 1;

	vcs* p_Gamma = (vcs*)_gamma;

	//int count = 1377; // 1377 = (5508*4) (gamma_size) / 16 (aligned)
	int count = BlockLen / 4; // (BlockLen*4) (gamma_size) / 16 (aligned)

	//p_Gamma += 1376;
	p_Gamma += count-1;

	// xmm5 contains all 0
	// Init Beta
	_xmm0 = *(pBetaPara+6);

	// load 4 gammas in a row
	_xmm6 = permutate<0xe4>(*p_Gamma);
	// store the initial
	*p_AlphaBeta = _xmm0;
	
	do{
			//r0+b0=b1
	 // xmm2 all beta h and l
		__asm
	  {
		  movdqa xmm0, _xmm0;
		  pshufb xmm0, BetaPara;
		  movdqa _xmm0, xmm0;
	  }

		_xmm1 = _xmm6;
		
		// gamma all
		__asm
	  {
		  movdqa xmm1, _xmm1;
		  pshufb xmm1, BetaPara+32 ;
		  movdqa _xmm1, xmm1;
	  }

		// hdd - alpha 1 t1; ldd - alpha 1 t2
		_xmm0 = saturated_add((vb)_xmm0,(vb)_xmm1);
		// ldd - alpha 1 t1; hdd - alpha 1 t2
		_xmm3 = permutate<0x4e>(_xmm0);

		// compute the max
		_xmm0 = smax((vb)_xmm0,(vb)_xmm3);

		// xmm2 - alpha 1
		// normalize
		_xmm4 = _xmm0;		 	
		
		//pshufb xmm4, xmm5 ; 	  // spread x0 to all xmm4
		__asm
	  {
		  movdqa xmm4, _xmm4;
		  movdqa xmm5, _xmm5;
		  pshufb xmm4, xmm5;
		  movdqa _xmm4, xmm4;
		  movdqa _xmm5, xmm5;
	  }
		// normalize
		_xmm0 = saturated_sub((vb)_xmm0,(vb)_xmm4);

		//pslldq xmm0, 0x08 ;
		__asm
		{
			movdqa xmm0, _xmm0;
			pslldq xmm0, 0x08;
			movdqa _xmm0, xmm0;
			
		}

		//r1+a1=a2
		// dup
		_xmm2 = _xmm0;
		// beta 1
		__asm
	  {
		  movdqa xmm2, _xmm2;
		  pshufb xmm2, BetaPara+16 ;
		  movdqa _xmm2, xmm2;
	  }

		// dup gamma
		_xmm1 = _xmm6;
		// gamma all
		__asm
	  {
		  movdqa xmm1, _xmm1;
		  pshufb xmm1, BetaPara+48 ;
		  movdqa _xmm1, xmm1;
	  }

		// hdd - alpha 2 t1
		_xmm2 = saturated_add((vb)_xmm2,(vb)_xmm1);
		_xmm3 = permutate<0x4e>(_xmm2);

		_xmm2 = smax((vb)_xmm2,(vb)_xmm3);

		// normalize
		_xmm4 = _xmm2;
		//pshufb xmm4, xmm5 ;
		__asm
	  {
		  movdqa xmm4, _xmm4;
		  movdqa xmm5, _xmm5;
		  pshufb xmm4, xmm5;
		  movdqa _xmm4, xmm4;
		  movdqa _xmm5, xmm5;
	  }
		// xmm0 - ldd - alpha 2
		_xmm2 = saturated_sub((vb)_xmm2,(vb)_xmm4);

		//pslldq xmm2, 0x08 ;
		__asm
		{
			movdqa xmm2, _xmm2;
			pslldq xmm2, 0x08;
			movdqa _xmm2, xmm2;
			
		}

		_xmm2 = interleave_high((vq)_xmm2,(vq)_xmm0);
		// store
		*(p_AlphaBeta-1) = _xmm2;
	
		//r2+a2=a3
		//pshufb xmm2, BetaPara ;
		__asm
	  {
		  movdqa xmm2, _xmm2;
		  pshufb xmm2, BetaPara ;
		  movdqa _xmm2, xmm2;
	  }

		//pshufb xmm6, BetaPara+64 ; // gamm all
		__asm
	  {
		  movdqa xmm6, _xmm6;
		  pshufb xmm6, BetaPara+64 ;
		  movdqa _xmm6, xmm6;
	  }

		//mb,mt;
		_xmm2 = saturated_add((vb)_xmm2,(vb)_xmm6);
		//mt,mb
		_xmm3 = permutate<0x4e>(_xmm2);

		_xmm2 = smax((vb)_xmm2,(vb)_xmm3);

		_xmm4 = _xmm2;
		//pshufb xmm4, xmm5 ;
		__asm
	  {
		  movdqa xmm4, _xmm4;
		  movdqa xmm5, _xmm5;
		  pshufb xmm4, _xmm5 ;
		  movdqa _xmm4, xmm4;
		  movdqa _xmm5, xmm5;
	  }
		_xmm2 = saturated_sub((vb)_xmm2,(vb)_xmm4);

		//pslldq xmm2, 0x08 ;
		__asm
		{
			movdqa xmm2, _xmm2;
			pslldq xmm2, 0x08;
			movdqa _xmm2, xmm2;
			
		}
		
		//r3+a3=a4
		_xmm0 = _xmm2;
		//pshufb xmm0, BetaPara+16 ;
		__asm
	  {
		  movdqa xmm0, _xmm0;
		  pshufb xmm0, BetaPara+16 ;
		  movdqa _xmm0, xmm0;
	  }

		// reload xmm6
		_xmm6 = *(p_Gamma-1);
		// dup
		_xmm1 = _xmm6;
		//pshufb xmm1, BetaPara+80 ;
		__asm
	  {
		  movdqa xmm1, _xmm1;
		  pshufb xmm1, BetaPara+80 ;
		  movdqa _xmm1, xmm1;
	  }

		//mb,mt
		_xmm0 = saturated_add((vb)_xmm0,(vb)_xmm1);
		//mt,mb
		_xmm3 = permutate<0x4e>(_xmm0);
		_xmm0 = smax((vb)_xmm0,(vb)_xmm3);

		_xmm4 = _xmm0;
		//pshufb xmm4, xmm5 ;
		__asm
	  {
		  movdqa xmm4, _xmm4;
		  movdqa xmm5, _xmm5;
		  pshufb xmm4, _xmm5;
		  movdqa _xmm4, xmm4;
		  movdqa _xmm5, xmm5;
	  }
		_xmm0 = _xmm4;
		//pslldq xmm0, 0x08 ;
		__asm
		{
			movdqa xmm0, _xmm0;
			pslldq xmm0, 0x08;
			movdqa _xmm0, xmm0;
			
		}

		_xmm0 = interleave_high((vq)_xmm0,(vq)_xmm2);
		*(p_AlphaBeta-2) = _xmm0;
		
		p_AlphaBeta -= 2;
		p_Gamma -= 1;
	}while(--count>0);

}

void NePoS(int BlockLen, char* _AlphaBeta, char* _gamma, char*_NePo)
{
	
	vcs _xmm0, _xmm1, _xmm2, _xmm3, _xmm4, _xmm5, _xmm6, _xmm7;
	vcs* pAlphaBeta = (vcs*)_AlphaBeta;
	vcs* pGamma = (vcs*)_gamma;
  
	vcs* p_NePo = (vcs*)_NePo;

	//int count = 1377; // 1377 = (5508*4) (gamma_size) / 16 (aligned)
	int count = BlockLen / 4; // (BlockLen*4) (gamma_size) / 16 (aligned)
	int offset = BlockLen / 4;

	vcs* pNePo = (vcs*)NePoPara;
	_xmm6 = *pNePo;
	_xmm7 = permutate<0xe4>(*(pNePo+1));

	do{
        //L
		// load alpha
		_xmm0 = *pAlphaBeta;
		// load beta
		//_xmm1 = (*(pAlphaBeta+2756));
		_xmm1 = (*(pAlphaBeta + ((offset + 1) * 2)));
		// load gamma
		_xmm2 = permutate<0xe4>(*pGamma);

        // dup
		_xmm3 = _xmm0;
		_xmm4 = _xmm1;
		_xmm5 = _xmm2;

		//beta
		 __asm
	   	{
	   		movdqa xmm1, _xmm1;
			movdqa xmm6, _xmm6;
	   		pshufb xmm1, xmm6  ;
			movdqa _xmm1, xmm1;
			movdqa _xmm6, xmm6;
       	}
	//gamma
		
		__asm
	  {
		  movdqa xmm2, _xmm2;
		  pshufb xmm2, NePoPara+32 ;
		  movdqa _xmm2, xmm2;
	  }
		        
        _xmm0 = saturated_add((vb)_xmm0,(vb)_xmm1);
		//alpha + beta +gamma
		_xmm0 = saturated_add((vb)_xmm0,(vb)_xmm2);

        // store
		*p_NePo = _xmm0;

		// save gamma
		_xmm2 = _xmm5;
	   

       __asm
	   	{
	   		movdqa xmm4, _xmm4;
			movdqa xmm7, _xmm7;
	   		pshufb xmm4, xmm7 ;
			movdqa _xmm4, xmm4;
			movdqa _xmm7, xmm7;
       	}
		
//        pshufb _xmm5, NePoPara+64 ;
		__asm
	  {
		  movdqa xmm5, _xmm5;
		  pshufb xmm5, NePoPara+64 ;
		  movdqa _xmm5, xmm5;
	  }
	
		_xmm3 = saturated_add((vb)_xmm3,(vb)_xmm4);
		_xmm3 = saturated_add((vb)_xmm3,(vb)_xmm5);

        // store
		//*(p_NePo+2754) = _xmm3;
		*(p_NePo + 2 * offset) = _xmm3;

        // High dword

		// load alpha
		_xmm0 = *(pAlphaBeta+1);
		// load beta
		//_xmm1 = *(pAlphaBeta+2757);
		_xmm1 = *(pAlphaBeta + 2 * offset + 3);

        // dup
		_xmm3 = _xmm0;
		_xmm4 = _xmm1;
		_xmm5 = _xmm2;

		//beta
		   __asm
	   	{
	   		movdqa xmm1, _xmm1;
			movdqa xmm6, _xmm6;
	   		pshufb xmm1, xmm6;
			movdqa _xmm1, xmm1;
			movdqa _xmm6, xmm6;
       	}
		
		//gamma
			__asm
	  {
		  movdqa xmm2, _xmm2;
		  pshufb xmm2, NePoPara+48 ;
		  movdqa _xmm2, xmm2;
	  }
		_xmm0 = saturated_add((vb)_xmm0,(vb)_xmm1);
		_xmm0 = saturated_add((vb)_xmm0,(vb)_xmm2);

        // store
		*(p_NePo+1) = _xmm0;
		
       // pshufb _xmm4, _xmm7 ;
		   __asm
	   	{
	   		movdqa xmm4, _xmm4;
			movdqa xmm7, _xmm7;
	   		pshufb xmm4, xmm7 ;
			movdqa _xmm4, xmm4;
			movdqa _xmm7, xmm7;
       	}

        //pshufb _xmm5, NePoPara+80 ;
		__asm
	  {
		  movdqa xmm5, _xmm5;
		  pshufb xmm5, NePoPara+80 ;
		  movdqa _xmm5, xmm5;
	  }
	
		_xmm3 = saturated_add((vb)_xmm3,(vb)_xmm4);
		_xmm3 = saturated_add((vb)_xmm3,(vb)_xmm5);
		

        // store
		//*(p_NePo+2755)= _xmm3;
		*(p_NePo + 2 * offset + 1) = _xmm3;

		pAlphaBeta+=2;
		p_NePo+=2;
		pGamma+=1;
	}while(--count>0);
}

void TnS(int BlockLen, char* _NePo, char* _tntp)
{
	vcs _xmm0, _xmm1, _xmm2, _xmm3, _xmm4, _xmm5, _xmm6, _xmm7;
	vcs *pNePo = (vcs*)_NePo;
	vcs* p_Tntp = (vcs*)_tntp;

	//int count = 344;
	int AlignedBlockLen = (BlockLen % 16 == 0) ? BlockLen : ((BlockLen / 16 + 1) * 16);
	int count = AlignedBlockLen/16-1;

	vcs* ptntp = (vcs*)tntpPara;
	_xmm7 = *ptntp;
	


	do{
		// a8,b8,c8,d8 -> a4,b4,c4,d4
		//1-a8-b8
		_xmm0 = *pNePo;
		//1-c8-d8
		_xmm4 = permutate<0xe4>(*(pNePo+1));
		// dup 
		_xmm1 = _xmm0;

		// 1-a4-c4-a8-c8
		_xmm0 = interleave_low((vi)_xmm0,(vi)_xmm4);
		
		// 1-b4-d4-b8-d8
		_xmm1 = interleave_high((vi)_xmm1,(vi)_xmm4);
		
		_xmm2 = _xmm0;
		// 1-a4-c4-b4-d4
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm1);
		// 1-a8-c8-b8-d8
		_xmm2 = interleave_high((vq)_xmm2,(vq)_xmm1);
		
		// get max
		_xmm0 = smax((vb)_xmm0,(vb)_xmm2);

		//e8,f8,g8,h8,->e4,f4,e4,h4,
		//1-e8-f8
		_xmm1 = *(pNePo+2);
		//1-g8-h8
		_xmm4 = permutate<0xe4>(*(pNePo+3));
		// dup 
		_xmm2 = _xmm1;

		// 1-e4-g4-e8-g8
		_xmm1 = interleave_low((vi)_xmm1,(vi)_xmm4);
		
		// 1-f4-h4-f8-h8
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm4);

		_xmm3 = _xmm1;
		// 1-e4-g4-f4-h4
		_xmm1 = interleave_low((vq)_xmm1,(vq)_xmm2);
		// 1-e8-g8-f8-h8
		_xmm3 = interleave_high((vq)_xmm3,(vq)_xmm2);


		// get max
		_xmm1 = smax((vb)_xmm1,(vb)_xmm3);

		// xmm0 vs. xmm1
		_xmm3 = _xmm0;
		
		// xmm0	a2-e2-a4-e4-c2-g2-c4-g4
		_xmm0 = interleave_low((vs)_xmm0,(vs)_xmm1);

		// xmm1	b2-f2-b4-f4-d2-h2-d4-h4 			   
		_xmm3 = interleave_high((vs)_xmm3,(vs)_xmm1);
		_xmm2 = _xmm0;
		// xmm0: a2-e2-b2-f2-a4-e4-b4-f4
		_xmm0 = interleave_low((vi)_xmm0,(vi)_xmm3);
		
		
		// xmm2: c2-g2-d2-h2-c4-g4-d4-h4
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm3);
		
		_xmm3 = _xmm0;
		// xmm0: a2-e2-b2-f2-c2-g2-d2-h2
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm2);
		// xmm3: a4-e4-b4-f4-c4-g4-d4-h4
		_xmm3 = interleave_high((vq)_xmm3,(vq)_xmm2);

		// get max 
		_xmm0 =smax((vb)_xmm0,(vb)_xmm3);

		//i8,j8,k8,l8,->i4,j4,k4,l4,
		//1-i8-j8
		_xmm1 = *(pNePo+4);
		//1-k8-l8
		_xmm4 = permutate<0xe4>(*(pNePo+5));
		// dup 
		_xmm2 = _xmm1;

		// 1-i4-k4-i8-k8
		_xmm1 = interleave_low((vi)_xmm1,(vi)_xmm4);
		
		// 1-j4-l4-j8-l8
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm4);
		
		_xmm3 = _xmm1;
		// 1-i4-k4-j4-l4
		_xmm1 = interleave_low((vq)_xmm1,(vq)_xmm2);
		// 1-i8-k8-j8-l8
		_xmm3 = interleave_high((vq)_xmm3,(vq)_xmm2);


		// get max
		_xmm1 = smax((vb)_xmm1,(vb)_xmm3);

		//1-m8-n8
		_xmm2 = *(pNePo+6);
		//1-o8-p8
		_xmm4 = permutate<0xe4>(*(pNePo+7));
		// dup 
		_xmm3 = _xmm2;

		// 1-m4-o4-m8-o8
		_xmm2 = interleave_low((vi)_xmm2,(vi)_xmm4);
		
		// 1-n4-p4-n8-p8
		_xmm3 = interleave_high((vi)_xmm3,(vi)_xmm4);
		
		_xmm5 = _xmm2;
		// 1-m4-o4-n4-p4
		_xmm2 = interleave_low((vq)_xmm2,(vq)_xmm3);
		// 1-m8-o8-n8-p8
		_xmm5 = interleave_high((vq)_xmm5,(vq)_xmm3);


		// get max
		_xmm2 = smax((vb)_xmm2,(vb)_xmm5);

		// xmm1 vs. xmm2
		_xmm3 = _xmm1;
		//  i2-m2-i4-m4-k2-o2-k4-o4
		_xmm1 = interleave_low((vs)_xmm1,(vs)_xmm2);
		//  j2-n2-j4-n4-l2-p2-l4-p4				  
		_xmm3 = interleave_high((vs)_xmm3,(vs)_xmm2);

		_xmm5 = _xmm1;
		//  i2-m2-j2-n2-i4-m4-j4-n4
		_xmm1 = interleave_low((vi)_xmm1,(vi)_xmm3);
		
		//  k2-o2-l2-p2-k4-o4-l4-p4
		_xmm5 = interleave_high((vi)_xmm5,(vi)_xmm3);
		
		_xmm2 = _xmm1;
		// xmm1: i2-m2-j2-n2-k2-o2-l2-p2
		_xmm1 = interleave_low((vq)_xmm1,(vq)_xmm5);
		// xmm2: i4-m4-j4-n4-k4-o4-l4-p4
		_xmm2 = interleave_high((vq)_xmm2,(vq)_xmm5);

		// get max
		_xmm1 = smax((vb)_xmm1,(vb)_xmm2);

		// 2 --> 1
		_xmm2 = _xmm0;

		_xmm0 = interleave_low((vb)_xmm0,(vb)_xmm1);
		_xmm2 = interleave_high((vb)_xmm2,(vb)_xmm1);

		_xmm3 = _xmm0;
		_xmm0 = interleave_low((vs)_xmm0,(vs)_xmm2);
		_xmm3 = interleave_high((vs)_xmm3,(vs)_xmm2);

		_xmm2 = _xmm0;
		_xmm0 = interleave_low((vi)_xmm0,(vi)_xmm3);
		
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm3);

		_xmm1 = _xmm0;
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm2);
		_xmm1 = interleave_high((vq)_xmm1,(vq)_xmm2);

		// get max
		_xmm0 = smax((vb)_xmm0,(vb)_xmm1);

		//pshufb xmm0, xmm7 ;  // shuffle to get right order
		 __asm
	   	{
	   		movdqa xmm0, _xmm0;			
			movdqa xmm7, _xmm7;
	   		pshufb xmm0, xmm7 ;
			movdqa _xmm0, xmm0;
			movdqa _xmm7, xmm7;
			
       	}
		*p_Tntp = _xmm0;
		p_Tntp += 1;
		pNePo += 8;
	}while(--count>0);	
	
		
}

void TpS(int BlockLen, char* _NePo, char* _tntp)
{
	//int count = 344;
	int AlignedBlockLen = (BlockLen % 16 == 0) ? BlockLen : ((BlockLen / 16 + 1) * 16);
	int count = AlignedBlockLen / 16 - 1;

 	vcs _xmm0, _xmm1, _xmm2, _xmm3, _xmm4, _xmm5, _xmm6, _xmm7;
	vcs* pNePo = (vcs*)_NePo;
	//pNePo += 2754;
	pNePo += BlockLen/2;
	vcs* pTntp = (vcs*)_tntp;
	//pTntp += 345;
	pTntp += count + 1;


	do{
		// a8,b8,c8,d8 -> a4,b4,c4,d4
		//1-a8-b8
		_xmm0 = *pNePo;
		//1-c8-d8
		_xmm4 = permutate<0xe4>(*(pNePo+1)); 
		// dup 
		_xmm1 = _xmm0;

		// 1-a4-c4-a8-c8
		_xmm0 = interleave_low((vi)_xmm0,(vi)_xmm4);
		// 1-b4-d4-b8-d8
		_xmm1 = interleave_high((vi)_xmm1,(vi)_xmm4);

		_xmm2 = _xmm0;
		// 1-a4-c4-b4-d4
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm1);
		// 1-a8-c8-b8-d8
		_xmm2 = interleave_high((vq)_xmm2,(vq)_xmm1);


		// get max
		_xmm0 = smax((vb)_xmm0,(vb)_xmm2);

		//e8,f8,g8,h8,->e4,f4,e4,h4,
		//1-e8-f8
		_xmm1 = *(pNePo+2);
		//1-g8-h8
		_xmm4 = permutate<0xe4>(*(pNePo+3));
		// dup 
		_xmm2 = _xmm1;

		// 1-e4-g4-e8-g8
		_xmm1 = interleave_low((vi)_xmm1,(vi)_xmm4);
		// 1-f4-h4-f8-h8
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm4);

		_xmm3 = _xmm1;
		// 1-e4-g4-f4-h4
		_xmm1 = interleave_low((vq)_xmm1,(vq)_xmm2);
		// 1-e8-g8-f8-h8
		_xmm3 = interleave_high((vq)_xmm3,(vq)_xmm2);


		// get max
		_xmm1 = smax((vb)_xmm1,(vb)_xmm3);

		// xmm0 vs. xmm1
		_xmm3 = _xmm0;
		// xmm0	a2-e2-a4-e4-c2-g2-c4-g4
		_xmm0 = interleave_low((vs)_xmm0,(vs)_xmm1);
		// xmm1	b2-f2-b4-f4-d2-h2-d4-h4 			   
		_xmm3 = interleave_high((vs)_xmm3,(vs)_xmm1);

		_xmm2 = _xmm0;
		// xmm0: a2-e2-b2-f2-a4-e4-b4-f4
		_xmm0 = interleave_low((vi)_xmm0,(vi)_xmm3);
		// xmm2: c2-g2-d2-h2-c4-g4-d4-h4
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm3);

		_xmm3 = _xmm0;
		// xmm0: a2-e2-b2-f2-c2-g2-d2-h2
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm2);
		// xmm3: a4-e4-b4-f4-c4-g4-d4-h4
		_xmm3 = interleave_high((vq)_xmm3,(vq)_xmm2);

		// get max
		_xmm0 = smax((vb)_xmm0,(vb)_xmm3);

		//i8,j8,k8,l8,->i4,j4,k4,l4,
		//1-i8-j8
		_xmm1 = *(pNePo+4);
		//1-k8-l8
		_xmm4 = permutate<0xe4>(*(pNePo+5));
		// dup 
		_xmm2 = _xmm1;

		// 1-i4-k4-i8-k8
		_xmm1 = interleave_low((vi)_xmm1,(vi)_xmm4);
		// 1-j4-l4-j8-l8
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm4);

		_xmm3 = _xmm1;
		// 1-i4-k4-j4-l4
		_xmm1 = interleave_low((vq)_xmm1,(vq)_xmm2);
		// 1-i8-k8-j8-l8
		_xmm3 = interleave_high((vq)_xmm3,(vq)_xmm2);
		
		// get max
		_xmm1 = smax((vb)_xmm1,(vb)_xmm3);

		//1-m8-n8
		_xmm2 = *(pNePo+6);
		//1-o8-p8
		_xmm4 = permutate<0xe4>(*(pNePo+7));
		// dup 
		_xmm3 = _xmm2;

		// 1-m4-o4-m8-o8
		_xmm2 = interleave_low((vi)_xmm2,(vi)_xmm4);
		// 1-n4-p4-n8-p8
		_xmm3 = interleave_high((vi)_xmm3,(vi)_xmm4);

		_xmm5 = _xmm2;
		// 1-m4-o4-n4-p4
		_xmm2 = interleave_low((vq)_xmm2,(vq)_xmm3);
		// 1-m8-o8-n8-p8
		_xmm5 = interleave_high((vq)_xmm5,(vq)_xmm3);

		// get max
		_xmm2 = smax((vb)_xmm2,(vb)_xmm5);

		// xmm1 vs. xmm2
		_xmm3 = _xmm1;
		//  i2-m2-i4-m4-k2-o2-k4-o4
		_xmm1 = interleave_low((vs)_xmm1,(vs)_xmm2);
		//  j2-n2-j4-n4-l2-p2-l4-p4				  
		_xmm3 = interleave_high((vs)_xmm3,(vs)_xmm2);

		_xmm5 = _xmm1;
		//  i2-m2-j2-n2-i4-m4-j4-n4
		_xmm1 = interleave_low((vi)_xmm1,(vi)_xmm3);
		//  k2-o2-l2-p2-k4-o4-l4-p4
		_xmm5 = interleave_high((vi)_xmm5,(vi)_xmm3);

		_xmm2 = _xmm1;
		// xmm1: i2-m2-j2-n2-k2-o2-l2-p2
		_xmm1 = interleave_low((vq)_xmm1,(vq)_xmm5);
		// xmm2: i4-m4-j4-n4-k4-o4-l4-p4
		_xmm2 = interleave_high((vq)_xmm2,(vq)_xmm5);

		// get max
		_xmm1 = smax((vb)_xmm1,(vb)_xmm2);

		// 2 --> 1
		_xmm2 = _xmm0;

		_xmm0 = interleave_low((vb)_xmm0,(vb)_xmm1);
		_xmm2 = interleave_high((vb)_xmm2,(vb)_xmm1);

		_xmm3 = _xmm0;
		_xmm0 = interleave_low((vs)_xmm0,(vs)_xmm2);
		_xmm3 = interleave_high((vs)_xmm3,(vs)_xmm2);

		_xmm2 = _xmm0;
		_xmm0 = interleave_low((vi)_xmm0,(vi)_xmm3);
		_xmm2 = interleave_high((vi)_xmm2,(vi)_xmm3);

		_xmm1 = _xmm0;
		_xmm0 = interleave_low((vq)_xmm0,(vq)_xmm2);
		_xmm1 = interleave_high((vq)_xmm1,(vq)_xmm2);

		// get max
		_xmm0 = smax((vb)_xmm0,(vb)_xmm1);

		// shuffle to get right order
		 __asm
	   	{
	   		movdqa xmm0, _xmm0;
			pshufb xmm0, tntpPara ;
			movdqa _xmm0, xmm0;
			}
		*pTntp = _xmm0;

		pTntp += 1;
		pNePo += 8;
	}while(--count>0);

}


void ExOS(int BlockLen, char* _tntp, char* extI, char* extO)
{
	vcs xmm0, xmm1, xmm2;
	vcs* pTntp = (vcs*)_tntp;
	vcs* pExtI = (vcs*)extI;
	vcs* pExtO = (vcs*)extO;
	//int count = 345;
	int AlignedBlockLen = (BlockLen % 16 == 0) ? BlockLen : ((BlockLen / 16 + 1) * 16);
	int count = AlignedBlockLen / 16;

	do{
		//xmm0 = permutate<0xe4>(*(pTntp+345));
		xmm0 = permutate<0xe4>(*(pTntp + AlignedBlockLen / 16));
		xmm1 = permutate<0xe4>(*pTntp);
		xmm2 = permutate<0xe4>(*pExtI);

		xmm0 = saturated_sub((vb)xmm0,(vb)xmm1);
		xmm0 = saturated_sub((vb)xmm0,(vb)xmm2);
		*pExtO = xmm0;

		pTntp += 1;
		pExtI += 1;
		pExtO += 1;
	}while(--count>0);

}
#endif