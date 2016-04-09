#include "RegisterRW.h"


// Take from Dut 2.0 code

HANDLE hDeviceHandle = NULL;


HANDLE GetDeviceHandle(LPCTSTR szDeviceName)
{
    HANDLE hDrv = CreateFile ( szDeviceName, 
        GENERIC_READ | GENERIC_WRITE, 
        0,  
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );
    return hDrv;
}


HRESULT _ReadRegister(HANDLE hDevice, ULONG Offset, PULONG val)
{
    ULONG OutLen = sizeof(DEV_RET) + sizeof(ULONG);
    PDEV_RET pOut = (PDEV_RET)malloc(OutLen);
	PULONG data = (PULONG) ((char *) pOut + sizeof(DEV_RET));
    ULONG nWritten = 0;
	HRESULT out;

    BOOL ret = DeviceIoControl ( hDevice, 
                    DUT_IO_CODE(CMD_RD_REG),
                    &Offset,
                    sizeof(Offset),
                    pOut,
                    OutLen,
                    &nWritten,
                    NULL);
    if (nWritten != OutLen)
    {
        //printf("Read register failed\n");
		out = E_FAIL;
    }
    else
    {
        //printf("Ret: 0x%08x\n", pOut->hResult);
        //printf("Content: 0x%08x\n", *data);
		*val = *data;
		out = pOut->hResult;
    }
    free(pOut);
	return out;
}

HRESULT _WriteRegister(HANDLE hDevice, ULONG Offset, ULONG Value)
{
    OFFSET_VALUE Input;
    PDEV_RET pOut = (PDEV_RET)malloc(sizeof(DEV_RET));
    BOOL ret;
    ULONG nWritten;
	HRESULT out;

	Input.Offset    = Offset;
    Input.Value     = Value;
    ret = DeviceIoControl ( 
            hDevice, 
            DUT_IO_CODE(CMD_WR_REG),
            &Input,
            sizeof(Input),
            pOut,
            sizeof(DEV_RET), 
            &nWritten,
            NULL);
    if (nWritten != sizeof(DEV_RET))
    {
        //printf("Write register failed, %d, %d\n", nWritten, ret);
		out = E_FAIL;
    }
    else
    {
        //printf("Ret:0x%08x\n", pOut->hResult);
		out = pOut->hResult;
    }
    free(pOut);
	return out;
}



HRESULT SetFilterParams(int unit, ulong addr, ulong val)
{
	HRESULT hres; 
    HANDLE hDevice = hDeviceHandle;

	/*
	hDevice = GetDeviceHandle(DEVNAME);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Can't find device\n");
		return E_FAIL;
    }
	*/
	
	// Write addr
    hres = _WriteRegister(hDevice, 0x500, addr);		
	if (hres != 0) 
	{ 
		printf("Register access error!\n");
		return hres;
	}

	// Write value
    _WriteRegister(hDevice, 0x504, val);		
	if (hres != 0) 
	{ 
		printf("Register access error!\n");
		return hres;
	}

	// Write enable
    _WriteRegister(hDevice, 0x508, (1 << unit));		
	if (hres != 0) 
	{ 
		printf("Register access error!\n");
		return hres;
	}

	// Write disable
    _WriteRegister(hDevice, 0x508, 0);		
	if (hres != 0) 
	{ 
		printf("Register access error!\n");
		return hres;
	}

	return hres;
}



HRESULT ReadRegister(ULONG Offset, PULONG val)
{
	HRESULT hres; 
    HANDLE hDevice = hDeviceHandle;

	/*
	hDevice = GetDeviceHandle(DEVNAME);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Can't find device\n");
		return E_FAIL;
    }
	*/

	hres =  _ReadRegister(hDevice, Offset, val);

	return hres;
}


HRESULT WriteRegister(ULONG Offset, ULONG Value)
{
	HRESULT hres; 
	HANDLE hDevice = hDeviceHandle;

	/*
	hDevice = GetDeviceHandle(DEVNAME);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("Can't find device\n");
		return E_FAIL;
    }
	*/

	hres =  _WriteRegister(hDevice, Offset, Value);

	return hres;
}

// To read and write radio registers use
// HRESULT SoraUWriteRadioRegister(ULONG RadioNo, ULONG Addr, ULONG  Value);
// HRESULT SoraUReadRadioRegister(ULONG RadioNo, ULONG Addr, ULONG* Value);
// from _user_mode_ext.c








// Set dnsample = TRUE to downsample the incoming 40MHz signal into the LTE frequency 30.72 MHz
void SetRXDownsampleParameters(bool dnsample)
{
	HRESULT hres;
	int dn_w1st_r16[96] = {22869, 12971, 3072, 25941, 16043, 6144, 29013, 19115, 9216, 32085, 22186, 12288, 2390, 25258, 15360, 5462, 28330, 18432, 8534, 31402, 21504, 11605, 1707, 24576, 14677, 4779, 27648, 17749, 7851, 30720, 20821, 10923, 1024, 23893, 13995, 4096, 26965, 17067, 7168, 30037, 20139, 10240, 342, 23210, 13312, 3414, 26282, 16384, 6486, 29354, 19456, 9558, 32426, 22528, 12629, 2731, 25600, 15701, 5803, 28672, 18773, 8875, 31744, 21845, 11947, 2048, 24917, 15019, 5120, 27989, 18091, 8192, 31061, 21163, 11264, 1366, 24234, 14336, 4438, 27306, 17408, 7510, 30378, 20480, 10582, 683, 23552, 13653, 3755, 26624, 16725, 6827, 29696, 19797, 9899, 32767};
	int dn_w2nd_r16[96] = {9898, 19796, 29695, 6826, 16724, 26623, 3754, 13652, 23551, 682, 10581, 20479, 30377, 7509, 17407, 27305, 4437, 14335, 24233, 1365, 11263, 21162, 31060, 8191, 18090, 27988, 5119, 15018, 24916, 2047, 11946, 21844, 31743, 8874, 18772, 28671, 5802, 15700, 25599, 2730, 12628, 22527, 32425, 9557, 19455, 29353, 6485, 16383, 26281, 3413, 13311, 23209, 341, 10239, 20138, 30036, 7167, 17066, 26964, 4095, 13994, 23892, 1023, 10922, 20820, 30719, 7850, 17748, 27647, 4778, 14676, 24575, 1706, 11604, 21503, 31401, 8533, 18431, 28329, 5461, 15359, 25257, 2389, 12287, 22185, 32084, 9215, 19114, 29012, 6143, 16042, 25940, 3071, 12970, 22868, 0};
	int dn_rVec_r16[96] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 15, 17, 18, 19, 21, 22, 23, 25, 26, 27, 28, 30, 31, 32, 34, 35, 36, 38, 39, 40, 41, 43, 44, 45, 47, 48, 49, 51, 52, 53, 54, 56, 57, 58, 60, 61, 62, 64, 65, 66, 68, 69, 70, 71, 73, 74, 75, 77, 78, 79, 81, 82, 83, 84, 86, 87, 88, 90, 91, 92, 94, 95, 96, 97, 99, 100, 101, 103, 104, 105, 107, 108, 109, 110, 112, 113, 114, 116, 117, 118, 120, 121, 122, 124};

	// 0x0200 - r_clock
	// 0x0201 - r_wanted
	// 0x0202 - r_dnsample
	// 0x0203 - use_sampler
	// 0x0204 - test_data

#define DNSAMPLE_TEST		0

#if DNSAMPLE_TEST
	// Test:
	// 0x0200 - r_clock
	hres = SetFilterParams(0, 0x0200, 95);
	// 0x0201 - r_wanted
	hres = SetFilterParams(0, 0x0201, 95);
	// 0x0202 - r_dnsample
	hres = SetFilterParams(0, 0x0202, 0);

	for (int i=0; i<96; i++)
	{
		hres = SetFilterParams(0, 0x0000 + i, 0);
		hres = SetFilterParams(0, 0x0080 + i, 32767);
		hres = SetFilterParams(0, 0x0100 + i, i);
	}
#else
	// 0x0200 - r_clock
	hres = SetFilterParams(0, 0x0200, 124);
	// 0x0201 - r_wanted
	hres = SetFilterParams(0, 0x0201, 95);
	// 0x0202 - r_dnsample
	hres = SetFilterParams(0, 0x0202, 0);

	for (int i=0; i<96; i++)
	{
		//hres = SetFilterParams(0, 0x0000 + i, 32767);
		//hres = SetFilterParams(0, 0x0080 + i, 0);
		hres = SetFilterParams(0, 0x0000 + i, dn_w1st_r16[i]);
		hres = SetFilterParams(0, 0x0080 + i, dn_w2nd_r16[i]);
		hres = SetFilterParams(0, 0x0100 + i, dn_rVec_r16[i]);
	}
#endif


	// 0x0203 - use_sampler
	hres = SetFilterParams(0, 0x0203, dnsample);
	// 0x0204 - test_data
	hres = SetFilterParams(0, 0x0204, 0);


	// We want to receive the full 40 MHz signal,
	// not the downsampled 20 MHz version
	// So we need to set reg 0x17 to 0
	if (dnsample)
	{
		SoraUWriteRadioRegister(TARGET_RADIO_RX, 0x17, 1);
	}
	else
	{
		SoraUWriteRadioRegister(TARGET_RADIO_RX, 0x17, 0);
	}

	// DEBUG: Use 40MHz signal for test
	SoraUWriteRadioRegister(TARGET_RADIO_RX, 0x17, 1);

}



// Execute to enable TX command queue that allows
// continuous TX without gaps between buffers
// Max queueSize is 63. Has to be larger than 0!
void SetContinuousTXParameters(ulong queueSize)
{
	HRESULT hres;
	ulong val;

	if (queueSize <= 0 || queueSize > 63)
	{
		printf("TX requests queue size is %d and has to be larger than 0 and smaller than 64!\nExiting...", queueSize);
		exit(1);
	}
	// Allow <queueSize> queues request in TX command queue
	// 
	hres = WriteRegister(0x050C, queueSize);
}





// Set upsample = TRUE to upsample the incoming signal on LTE frequency 30.72 MHz into the outgoing 40MHz clock
void SetTXUpsampleParameters(bool upsample)
{
	HRESULT hres;
	int up_w1st_r16[125] = {32767, 7602, 15204, 22806, 30408, 5243, 12845, 20447, 28049, 2884, 10486, 18088, 25690, 525, 8127, 15729, 23331, 30933, 5767, 13369, 20971, 28573, 3408, 11010, 18612, 26214, 1049, 8651, 16253, 23855, 31457, 6292, 13894, 21496, 29098, 3933, 11534, 19136, 26738, 1573, 9175, 16777, 24379, 31981, 6816, 14418, 22020, 29622, 4457, 12059, 19661, 27263, 2098, 9700, 17301, 24903, 32505, 7340, 14942, 22544, 30146, 4981, 12583, 20185, 27787, 2622, 10224, 17826, 25428, 263, 7865, 15467, 23068, 30670, 5505, 13107, 20709, 28311, 3146, 10748, 18350, 25952, 787, 8389, 15991, 23593, 31195, 6030, 13632, 21234, 28835, 3670, 11272, 18874, 26476, 1311, 8913, 16515, 24117, 31719, 6554, 14156, 21758, 29360, 4195, 11797, 19399, 27001, 1835, 9437, 17039, 24641, 32243, 7078, 14680, 22282, 29884, 4719, 12321, 19923, 27525, 2360, 9962, 17564, 25166};
	int up_w2nd_r16[125] = {0, 25165, 17563, 9961, 2359, 27524, 19922, 12320, 4718, 29883, 22281, 14679, 7077, 32242, 24640, 17038, 9436, 1834, 27000, 19398, 11796, 4194, 29359, 21757, 14155, 6553, 31718, 24116, 16514, 8912, 1310, 26475, 18873, 11271, 3669, 28834, 21233, 13631, 6029, 31194, 23592, 15990, 8388, 786, 25951, 18349, 10747, 3145, 28310, 20708, 13106, 5504, 30669, 23067, 15466, 7864, 262, 25427, 17825, 10223, 2621, 27786, 20184, 12582, 4980, 30145, 22543, 14941, 7339, 32504, 24902, 17300, 9699, 2097, 27262, 19660, 12058, 4456, 29621, 22019, 14417, 6815, 31980, 24378, 16776, 9174, 1572, 26737, 19135, 11533, 3932, 29097, 21495, 13893, 6291, 31456, 23854, 16252, 8650, 1048, 26213, 18611, 11009, 3407, 28572, 20970, 13368, 5766, 30932, 23330, 15728, 8126, 524, 25689, 18087, 10485, 2883, 28048, 20446, 12844, 5242, 30407, 22805, 15203, 7601};
	int up_update_r16[125] = {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0};
	


	// We need to use 16-bit transmissions for TX upsampling
	SoraUWriteRadioRegister(TARGET_RADIO_TX, 0x16, 1);



	#define UPSAMPLE_TEST		0

	// ***** Set parameters for upsampler


	// 0x0201 - r_wanted
	hres = SetFilterParams(1, 0x0201, 124);

#if UPSAMPLE_TEST
	for (int i=0; i<125; i++)
	{
		hres = SetFilterParams(1, 0x0000 + i, 32767);
		hres = SetFilterParams(1, 0x0080 + i, 0);
		hres = SetFilterParams(1, 0x0100 + i, 1);
	}
#else
	for (int i=0; i<125; i++)
	{
		hres = SetFilterParams(1, 0x0000 + i, up_w1st_r16[i]);
		hres = SetFilterParams(1, 0x0080 + i, up_w2nd_r16[i]);
		hres = SetFilterParams(1, 0x0100 + i, up_update_r16[i]);
	}
#endif

	// 0x0203 - use_sampler
	hres = SetFilterParams(1, 0x0203, upsample);
	// 0x0204 - test_data
	hres = SetFilterParams(1, 0x0204, 0);
}





// Set RXTXSync type
// 0 - RX is entirely from the radio
// 1 - LSB of RX is copied over from TX
// 2 - The entire RX is copied over from TX
// Note: Use of TXRXSync (type != 0) will prevent TX unless RX is working at the same time!
void SetTXRXSync(ulong type)
{
	HRESULT hres;
	hres = WriteRegister(0x0510, type);
}



