//#include <bb/complex.h>
//#include "complex.h"
#include <vector128.h>
//#include <bb/basic.h>
#include "Demodulate.h"

#include <types.h>


#define Times 14400
#define RightMov 7

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


A16 short DemodulatePara[40]={1024,1024,1024,1024,1024,1024,1024,1024,
                             1,1,1,1,1,1,1,1,
                             -1024,-1024,-1024,-1024,-1024,-1024,-1024,-1024,
                             64,64,64,64,64,64,64,64,
                             512,1540,2568,3596,0,0,0,0};//0,2,4,6,8,10,12,14,0,0,0,0,0,0,0,0

void __ext_edemod(int8* output, int __unused_2, complex16 * input, int __unused_1)
{
	Demodulate((COMPLEX16*) input, (char *)output);
}


void Demodulate(COMPLEX16 * InfoOut , char * DeModOut)
{
	int i,j;
	vcs _xmm0,_xmm1,_xmm2,xmm3,xmm4,xmm5,xmm6,xmm7;
	vcs* pInfoOut;// = (vcs*)InfoOut;
	vcs* pDeModOut;// = (vcs*)DeModOut;
	vs *tPara = (vs*)DemodulatePara;	
     
	for (i=0; i<1; i++)
	{
		int count = 3600;
		pInfoOut = (vcs*)(InfoOut+i*14400);
		pDeModOut = (vcs*)(DeModOut+i*57600);
		xmm7 = __0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF<vcs>();
		xmm6 = xor(xmm6,xmm6);
	
		do
		{
			_xmm0 = *pInfoOut;
			_xmm1 = permutate<0xe4>(_xmm0);
			_xmm1 = xor(_xmm1,xmm7);
			_xmm1 = saturated_add((vs)_xmm1,*(tPara+1));
			_xmm2 = permutate<0xe4>(_xmm0);
			_xmm2 = is_great((vs)_xmm2,*(tPara));
			xmm3 = permutate<0xe4>(_xmm1);
			xmm3 = and(xmm3,_xmm2);//(x>1024:-x; x<=1024 : 0)
			_xmm2 = and(_xmm2,(vcs)*tPara);//(x>1024:1024;x<=1024:0000)
			_xmm1 = saturated_add((vs)_xmm1,(vs)xmm3);
			_xmm1 = saturated_add((vs)_xmm1,(vs)_xmm2);//(x>1024:1024-2x;x<=1024:-x)
			_xmm2 = permutate<0xe4>(_xmm0);
			_xmm2 = is_great((vs)_xmm2,*(tPara+2));
			_xmm2 = xor(_xmm2,xmm7);//(x<-1024:FFFF;x>=-1024:0000)
			xmm3 = permutate<0xe4>(_xmm1);
			xmm3 = and(xmm3,_xmm2);//(x<-1024:-x;x>=-1024:0)
			_xmm2 = and(_xmm2,(vcs)*(tPara+2));//(x<-1024:-1024;x>=-1024:0000)
			_xmm1 = saturated_add((vs)_xmm1,(vs)xmm3);
			_xmm1 = saturated_add((vs)_xmm1,(vs)_xmm2);//(x>1024:1024-2x;-1024<x<1024:-x;x<-1024:-1024-2x;)
        
			_xmm2 = permutate<0xe4>(_xmm0);
			_xmm2 = is_great((vs)_xmm2,(vs)xmm6);
			_xmm2 = xor(_xmm2,xmm7);//(x<0:FFFF;x>=0:0)
			xmm3 = permutate<0xe4>(_xmm2);
			xmm3 = and(xmm3,(vcs)*(tPara+1));//(x<0:1;x>=0:0)
			_xmm0 = xor(_xmm0,_xmm2);
			_xmm0 = saturated_add((vs)_xmm0,(vs)xmm3);	//(x<0:-x;x>=0:x)
			_xmm0 = saturated_sub((vs)_xmm0,*tPara);//(x<0:-x-1024;x>=0:x-1024)
			_xmm0 = saturated_sub((vs)_xmm0,*(tPara+3));
			_xmm1 = saturated_sub((vs)_xmm1,*(tPara+3));
			_xmm0 = shift_right(_xmm0,7);
			_xmm1 = shift_right(_xmm1, 7);
			//pshufb _xmm0,DemodulatePara+64;
			__asm
			{
        		movdqa xmm0, _xmm0;
				pshufb xmm0,DemodulatePara+64;
				movdqa _xmm0, xmm0;
			}
			//pshufb _xmm1,DemodulatePara+64;
			__asm
			{
        		movdqa xmm1, _xmm1;
				pshufb xmm1,DemodulatePara+64;
				movdqa _xmm1, xmm1;
			}
			
			_xmm1 = interleave_low((vs)_xmm1,(vs)_xmm0);
        
			
			*pDeModOut = _xmm1;
        	pDeModOut += 1;
			pInfoOut += 1;
			
		}while(--count>0);
	}

	return;
}