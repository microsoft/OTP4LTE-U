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
#include "mac_msg_parsing.h"

// L2(MAC)-level tests


//TEST_METHOD(bytes_to_hex_works)
int bytes_to_hex_works()
{
	char str[4096];
	const char *cstr = (char *)str;
	unsigned char bytes[2048];
	int len;
	int error = 0;

	bytes[0] = 0;
	bytes[1] = 0;
	len = bytes_to_hex(str, 4096, bytes, 1);
	if (len != 2 || strcmp(cstr, "00") != 0)
	{
		printf("Test bytes_to_hex_works: (\"00\", { 0 }) failed\n");
		error = 1;
	}

	bytes[0] = 1;
	len = bytes_to_hex(str, 4096, bytes, 1);
	if (len != 2 || strcmp(cstr, "01") != 0)
	{
		printf("Test bytes_to_hex_works: (\"01\", { 1 }) failed\n");
		error = 1;
	}

	bytes[0] = 15;
	len = bytes_to_hex(str, 4096, bytes, 1);
	if (len != 2 || strcmp(cstr, "0f") != 0)
	{
		printf("Test bytes_to_hex_works: (\"0f\", { 15 }) failed\n");
		error = 1;
	}

	bytes[0] = 255;
	len = bytes_to_hex(str, 4096, bytes, 1);
	if (len != 2 || strcmp(cstr, "ff") != 0)
	{
		printf("Test bytes_to_hex_works: (\"ff\", { 255 }) failed\n");
		error = 1;
	}

	bytes[0] = 0;
	len = bytes_to_hex(str, 4096, bytes, 2);
	if (len != 4 || strcmp(cstr, "0000") != 0)
	{
		printf("Test bytes_to_hex_works: (\"0000\", { 0, 0 }) failed\n");
		error = 1;
	}

	bytes[0] = 1;
	len = bytes_to_hex(str, 4096, bytes, 2);
	if (len != 4 || strcmp(cstr, "0100") != 0)
	{
		printf("Test bytes_to_hex_works: (\"0100\", { 1, 0 }) failed\n");
		error = 1;
	}

	return error;
}


//TEST_METHOD(hex_to_bytes_works)
int hex_to_bytes_works()
{
	unsigned char b[20];
	unsigned char bytes[2048];
	int len;
	int error = 0;

	memset(b, 0, 20);
	len = hex_to_bytes(bytes, 2048, "00");
	if (len != 1 || memcmp(bytes, b, len))
	{
		printf("Test hex_to_bytes_works: (\"00\", { 0 }) failed\n");
		error = 1;
	}

	b[0] = 1;
	len = hex_to_bytes(bytes, 2048, "01");
	if (len != 1 || memcmp(bytes, b, len))
	{
		printf("Test hex_to_bytes_works: (\"01\", { 1 }) failed\n");
		error = 1;
	}

	b[0] = 15;
	len = hex_to_bytes(bytes, 2048, "0f");
	if (len != 1 || memcmp(bytes, b, len))
	{
		printf("Test hex_to_bytes_works: (\"0f\", { 15 }) failed\n");
		error = 1;
	}

	b[0] = 255;
	len = hex_to_bytes(bytes, 2048, "ff");
	if (len != 1 || memcmp(bytes, b, len))
	{
		printf("Test hex_to_bytes_works: (\"ff\", { 255 }) failed\n");
		error = 1;
	}

	b[0] = 0;
	b[1] = 0;
	len = hex_to_bytes(bytes, 2048, "0000");
	if (len != 2 || memcmp(bytes, b, len))
	{
		printf("Test hex_to_bytes_works: (\"0000\", { 0, 0 }) failed\n");
		error = 1;
	}

	b[0] = 1;
	len = hex_to_bytes(bytes, 2048, "0100");
	if (len != 2 || memcmp(bytes, b, len))
	{
		printf("Test hex_to_bytes_works: (\"0100\", { 1, 0 }) failed\n");
		error = 1;
	}

	return error;
}


//TEST_METHOD(mac_decode_strips_header)
int mac_decode_strips_header()
{
	unsigned char bytes[2048];
	unsigned char bytes2[2048];
	int len;
	int len2;
	int error = 0;

	sdu out[64];
	int len_out;


	len = hex_to_bytes(bytes, 2048, "");
	len_out = mac_decode(out, 64, bytes, len);
	if (len_out != 0)
	{
		printf("Test mac_decode_strips_header <empty> failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "1f");
	len_out = mac_decode(out, 64, bytes, len);
	if (len_out != 0)
	{
		printf("Test mac_decode_strips_header 1f failed\n");
		error = 1;
	}

	// Captured from dongle.
	len = hex_to_bytes(bytes, 2048, "20061f500a5024b3a6");
	len_out = mac_decode(out, 64, bytes, len);
	len = hex_to_bytes(bytes, 2048, "500a5024b3a6");
	if (len_out != 1 || out[0].lcid != 0 || out[0].len != len || memcmp(bytes, out[0].bytes, len) != 0)
	{
		printf("Test mac_decode_strips_header 20061f500a5024b3a6 failed\n");
		error = 1;
	}

	// From http://www.sharetechnote.com/html/MAC_LTE.html
	// Had to change length from 27 to 2f to make sense, think there was an error on his page
	len = hex_to_bytes(bytes, 2048, "3d212f1f00a000002020000000260741010bf60000000000000000000002e06000050201d011d0190000025200000000005c080200000000");
	len_out = mac_decode(out, 64, bytes, len);
	len = hex_to_bytes(bytes, 2048, "a000002020000000260741010bf60000000000000000000002e06000050201d011d0190000025200000000005c0802");
	if (len_out != 1 || out[0].lcid != 1 || out[0].len != len || memcmp(bytes, out[0].bytes, len) != 0)
	{
		printf("Test mac_decode_strips_header 3d212f1f00a000002020000000260741010bf6... failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "3d210922111f00a00404100040ebc014a000004800e0e860006a401840000000000000000000000000");
	len_out = mac_decode(out, 64, bytes, len);
	len = hex_to_bytes(bytes, 2048, "a00404100040ebc014");
	len2 = hex_to_bytes(bytes2, 2048, "a000004800e0e860006a40184000000000");
	if (len_out != 2 || out[0].lcid != 1 || out[0].len != len || memcmp(bytes, out[0].bytes, len) != 0 ||
		out[1].lcid != 2 || out[1].len != len2 || memcmp(bytes2, out[1].bytes, len2) != 0)
	{
		printf("Test mac_decode_strips_header 3d210922111f00a00404100040ebc014a... failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "031804509ac2bc25c2f0bb0ba0b682f09a50970970bc26942f0ab0ba0bc2f0bc2f093a092709a50bc26942da0970ae824e82da0ab0b70b50bc2ba09a4c2d42750ab0bc2ec24d42f0bc2f0970bc26942f0970bc25c2bc2a4c2f0ab0bc2f0b682f0ab0ab09a50bc2da0bc2f0bc2ec2d42f0bc2ac2ac247004414f0b682f0bc25c2ba0ab0bc2f0bc25c2da0bc2f0ba0bc2f0b682da0bc269424e825c2f0bc2f0ae825c2f0bc2da0b50bc2f0bc2f0bc25c2d42f09d30bc2ec2d42f0bc2f0bc2ec2d425c26942f0bc2f0bc2f09a50bc2f0ab0bc2ac2f0bc2f0bc2f0ab09a50b682f0bc2");
	len_out = mac_decode(out, 64, bytes, len);
	len = hex_to_bytes(bytes, 2048, "1804509ac2bc25c2f0bb0ba0b682f09a50970970bc26942f0ab0ba0bc2f0bc2f093a092709a50bc26942da0970ae824e82da0ab0b70b50bc2ba09a4c2d42750ab0bc2ec24d42f0bc2f0970bc26942f0970bc25c2bc2a4c2f0ab0bc2f0b682f0ab0ab09a50bc2da0bc2f0bc2ec2d42f0bc2ac2ac247004414f0b682f0bc25c2ba0ab0bc2f0bc25c2da0bc2f0ba0bc2f0b682da0bc269424e825c2f0bc2f0ae825c2f0bc2da0b50bc2f0bc2f0bc25c2d42f09d30bc2ec2d42f0bc2f0bc2ec2d425c26942f0bc2f0bc2f09a50bc2f0ab0bc2ac2f0bc2f0bc2f0ab09a50b682f0bc2");
	if (len_out != 1 || out[0].lcid != 3 || out[0].len != len || memcmp(bytes, out[0].bytes, len) != 0)
	{
		printf("Test mac_decode_strips_header 031804509ac2bc25c2f0bb0ba0b682f09a509709... failed\n");
		error = 1;
	}

	/*
	for (int i = 0; i < len_out; i++)
	{
	bytes_to_hex(str, out[i].bytes, out[i].len);
	printf("lcid: %d, size: %d, msg:%s\n", out[i].lcid, out[i].len, str);
	}
	*/


	return error;

}


//TEST_METHOD(mac_encode_adds_header)
int mac_encode_adds_header()
{
	unsigned char bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;

	sdu sdus[3];
	int len_out;

	len_out = mac_encode(bytes, 2048, sdus, 0);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "") != 0)
	{
		printf("Test mac_encode_adds_header: <empty> failed\n");
		error = 1;
	}


	sdus[0].lcid = 0;
	sdus[0].len = hex_to_bytes(sdus[0].bytes, 2048, "");
	len_out = mac_encode(bytes, 2048, sdus, 1);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "20001f") != 0)
	{
		printf("Test mac_encode_adds_header: 20001f failed\n");
		error = 1;
	}


	sdus[0].lcid = 0;
	sdus[0].len = hex_to_bytes(sdus[0].bytes, 2048, "00");
	len_out = mac_encode(bytes, 2048, sdus, 1);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "20011f00") != 0)
	{
		printf("Test mac_encode_adds_header: 20011f00 failed\n");
		error = 1;
	}


	sdus[0].lcid = 0;
	sdus[0].len = hex_to_bytes(sdus[0].bytes, 2048, "01");
	len_out = mac_encode(bytes, 2048, sdus, 1);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "20011f01") != 0)
	{
		printf("Test mac_encode_adds_header: 20011f01 failed\n");
		error = 1;
	}


	sdus[0].lcid = 0;
	sdus[0].len = hex_to_bytes(sdus[0].bytes, 2048, "00");
	sdus[1].lcid = 1;
	sdus[1].len = hex_to_bytes(sdus[1].bytes, 2048, "00");
	len_out = mac_encode(bytes, 2048, sdus, 2);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "200121011f0000") != 0)
	{
		printf("Test mac_encode_adds_header: 200121011f0000 failed\n");
		error = 1;
	}


	sdus[0].lcid = 1;
	sdus[0].len = hex_to_bytes(sdus[0].bytes, 2048, "00");
	sdus[1].lcid = 0;
	sdus[1].len = hex_to_bytes(sdus[1].bytes, 2048, "00");
	len_out = mac_encode(bytes, 2048, sdus, 2);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "210120011f0000") != 0)
	{
		printf("Test mac_encode_adds_header: 210120011f0000 failed\n");
		error = 1;
	}


	sdus[0].lcid = 0;
	sdus[0].len = hex_to_bytes(sdus[0].bytes, 2048, "500a5024b3a6");
	len_out = mac_encode(bytes, 2048, sdus, 1);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "20061f500a5024b3a6") != 0)
	{
		printf("Test mac_encode_adds_header: 20061f500a5024b3a6 failed\n");
		error = 1;
	}


	sdus[0].lcid = 1;
	sdus[0].len = hex_to_bytes(sdus[0].bytes, 2048, "a00404100040ebc014");
	sdus[1].lcid = 2;
	sdus[1].len = hex_to_bytes(sdus[1].bytes, 2048, "a000004800e0e860006a40184000000000");
	len_out = mac_encode(bytes, 2048, sdus, 2);
	bytes_to_hex(str, 1000, bytes, len_out);
	if (strcmp(cstr, "210922111fa00404100040ebc014a000004800e0e860006a40184000000000") != 0)
	{
		printf("Test mac_encode_adds_header: 210902a00404100040ebc01... failed\n");
		error = 1;
	}


	return error;
}


//TEST_METHOD(rlc_decode_strips_header)
int rlc_decode_strips_header()
{
	unsigned char bytes[2048];
	unsigned char out[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, len_out;
	unsigned short rlc_sn = 0;
	unsigned char rlc_poll_flag = 0;
	unsigned char rlc_sdu_buffer[MAX_MAC_MSG_SIZE];
	unsigned char rlc_sdu_buffer_index;


	len = hex_to_bytes(bytes, 2048, "");
	len_out = rlc_decode(out, 2048, 0, bytes, len, &rlc_sn, &rlc_poll_flag, rlc_sdu_buffer, &rlc_sdu_buffer_index);
	if (len_out != 0)
	{
		printf("Test rlc_decode_strips_header: <empty> failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "500a5024b3a6");
	len_out = rlc_decode(out, 2048, 0, bytes, len, &rlc_sn, &rlc_poll_flag, rlc_sdu_buffer, &rlc_sdu_buffer_index);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "500a5024b3a6") != 0)
	{
		printf("Test rlc_decode_strips_header: 500a5024b3a6 failed\n");
		error = 1;
	}

	// From http://www.sharetechnote.com/html/RLC_LTE.html
	len = hex_to_bytes(bytes, 2048, "0004");	// STATUS
	len_out = rlc_decode(out, 2048, 1, bytes, len, &rlc_sn, &rlc_poll_flag, rlc_sdu_buffer, &rlc_sdu_buffer_index);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "") != 0)
	{
		printf("Test rlc_decode_strips_header: 0004 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "a00101480160ea611479e5cbce4d6ad68000000000");
	len_out = rlc_decode(out, 2048, 1, bytes, len, &rlc_sn, &rlc_poll_flag, rlc_sdu_buffer, &rlc_sdu_buffer_index);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "01480160ea611479e5cbce4d6ad68000000000") != 0)
	{
		printf("Test rlc_decode_strips_header: a00101480160ea611479e5cbce4d6ad68000000000 failed\n");
		error = 1;
	}

	// TODO: two RLC segments
	//ans = rlc_decode(3, hex_to_bytes("1ce1068fcc4823e990da18d22b1ba3d95c72e8d771a951a982781780dc9b1876179918487f7722b98e79daa9585ac86e1c93f91d64c45856460cb5a30c5eb324498ff051fcd7a0f629a35173c2319ef84b0f0734b106a8e179e0e0ddc7c8fc235208d8f448ed9a7010efdd1c8054450002500e620000801195c90101c9370101c93804e704d2023c9b6247004411e6f0b01c8033a52c12c2c8351692616812070d81ccc729c921a1b9946c2522e328781a6ae35a501b138333c657114d5b195508312675fa4b42fb84ff2b1be222b39c670720ac24070d896b4d072457bc94cf"));
	//Assert::AreEqual("01480160ea611479e5cbce4d6ad68000000000", bytes_to_hex(ans).c_str());

	return error;
}


//TEST_METHOD(rlc_encode_adds_header)
int rlc_encode_adds_header()
{
	unsigned char bytes[2048];
	unsigned char out[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, len_out;
	unsigned int sn;

	len = hex_to_bytes(bytes, 2048, "");
	sn = 0;
	len_out = rlc_encode(out, 2048, 0, &sn, bytes, len);
	if (len_out != 0 || sn != 0)
	{
		printf("Test rlc_encode_adds_header: <empty> failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "500a5024b3a6");
	sn = 0;
	len_out = rlc_encode(out, 2048, 0, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 0 || strcmp(cstr, "500a5024b3a6") != 0)
	{
		printf("Test rlc_encode_adds_header: 500a5024b3a6 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "");
	sn = 0;
	len_out = rlc_encode(out, 2048, 1, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 1 || strcmp(cstr, "a000") != 0)
	{
		printf("Test rlc_encode_adds_header: lcid=1 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "");
	sn = 0;
	len_out = rlc_encode(out, 2048, 2, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 1 || strcmp(cstr, "a000") != 0)
	{
		printf("Test rlc_encode_adds_header: lcid=2 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "");
	sn = 0;
	len_out = rlc_encode(out, 2048, 3, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 1 || strcmp(cstr, "00") != 0)
	{
		printf("Test rlc_encode_adds_header: lcid=3 failed\n");
		error = 1;
	}

	return error;
}


//TEST_METHOD(pdcp_decode_strips_header)
int pdcp_decode_strips_header()
{
	unsigned char bytes[2048];
	unsigned char out[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, len_out;

	len = hex_to_bytes(bytes, 2048, "");
	len_out = pdcp_decode(out, 2048, 0, bytes, len);
	if (len_out != 0)
	{
		printf("Test pdcp_decode_strips_header: <empty> failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "500a5024b3a6");
	len_out = pdcp_decode(out, 2048, 0, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "500a5024b3a6") != 0)
	{
		printf("Test pdcp_decode_strips_header: 500a5024b3a6 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "000000000000");
	len_out = pdcp_decode(out, 2048, 1, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "00") != 0)
	{
		printf("Test pdcp_decode_strips_header: 000000000000 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "000100000000");
	len_out = pdcp_decode(out, 2048, 2, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "01") != 0)
	{
		printf("Test pdcp_decode_strips_header: 000100000000 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "0000");
	len_out = pdcp_decode(out, 2048, 3, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "") != 0)
	{
		printf("Test pdcp_decode_strips_header: 0000 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "010001020304");
	len_out = pdcp_decode(out, 2048, 3, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "") != 0)
	{
		printf("Test pdcp_decode_strips_header: 010001020304 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "8000");
	len_out = pdcp_decode(out, 2048, 3, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (strcmp(cstr, "00") != 0)
	{
		printf("Test pdcp_decode_strips_header: 8000 failed\n");
		error = 1;
	}

	return error;
}


//TEST_METHOD(pdcp_encode_adds_header)
int pdcp_encode_adds_header()
{
	unsigned char bytes[2048];
	unsigned char out[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, len_out;
	unsigned int sn = 0;

	len = hex_to_bytes(bytes, 2048, "");
	sn = 0;
	len_out = pdcp_encode(out, 2048, 0, &sn, bytes, len);
	if (sn != 0 || len_out != 0)
	{
		printf("Test pdcp_encode_adds_header: <empty> failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "500a5024b3a6");
	sn = 0;
	len_out = pdcp_encode(out, 2048, 0, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 0 || strcmp(cstr, "500a5024b3a6") != 0)
	{
		printf("Test pdcp_encode_adds_header: 500a5024b3a6 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "");
	sn = 0;
	len_out = pdcp_encode(out, 2048, 1, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 1 || strcmp(cstr, "0000000000") != 0)
	{
		printf("Test pdcp_encode_adds_header: 0000000000 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "ff");
	sn = 0;
	len_out = pdcp_encode(out, 2048, 2, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 1 || strcmp(cstr, "00ff00000000") != 0)
	{
		printf("Test pdcp_encode_adds_header: 00ff00000000 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "");
	sn = 0;
	len_out = pdcp_encode(out, 2048, 3, &sn, bytes, len);
	bytes_to_hex(str, 1000, out, len_out);
	if (sn != 1 || strcmp(cstr, "80") != 0)
	{
		printf("Test pdcp_encode_adds_header: 80 failed\n");
		error = 1;
	}


	return error;
}


//TEST_METHOD(rrc_handles_rrcconnectionrequest)
int rrc_handles_rrcconnectionrequest()
{
	unsigned char bytes[2048];
	char str[1000];
	char str2[1000];
	const char *cstr = (char *)str;
	const char *cstr2 = (char *)str2;
	sdu sdus[3];
	int error = 0;
	int len, len_out;

	len = hex_to_bytes(bytes, 2048, "500a5024b3a6");
	len_out = rrc(sdus, 3, 0, bytes, len);
	if (len_out != 2)
	{
		printf("Test rrc_handles_rrcconnectionrequest: 500a5024b3a6 failed len_out\n");
		error = 1;
	}
	else
	{
		bytes_to_hex(str, 1000, sdus[0].bytes, sdus[0].len);
		bytes_to_hex(str2, 1000, sdus[1].bytes, sdus[1].len);
		if (sdus[0].lcid != 28 || strcmp(cstr, "500a5024b3a6") != 0 ||
			sdus[1].lcid != 0 || strcmp(str2, "60109b0030000b00") != 0)
		{
			printf("Test rrc_handles_rrcconnectionrequest: 500a5024b3a6 failed\n");
			error = 1;
		}
	}

	return error;
}


//TEST_METHOD(end_to_end_request_to_setup)
int end_to_end_request_to_setup()
{
	unsigned char bytes[2048], tx_bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, tx_len;

	reset_state();

	len = hex_to_bytes(bytes, 2048, "20061f500a5024b3a6");

	tx_len = end_to_end(bytes, len, tx_bytes, 2048);

	bytes_to_hex(str, 1000, tx_bytes, tx_len);

	if (strcmp(cstr, "3c20081f500a5024b3a660109b0030000b00") != 0)
	{
		printf("Test end_to_end_request_to_setup failed\n");
		error = 1;
	}

	return error;
}

int end_to_end_setupcomplete_to_identity_request()
{
	unsigned char bytes[2048], tx_bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, tx_len;

	reset_state();

	// First fragment should trigger Identity Request (with our current hack to get around lack of reassembly).
	len = hex_to_bytes(bytes, 2048, "3d010b8800002000720e82e4101220202064a8ed3005c1c00042");
	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "") != 0)
	{
		printf("Test end_to_end_setupcomplete_to_identity_request first fragment failed\n");
		error = 1;
	}

	// Second fragment should cause no output.
	len = hex_to_bytes(bytes, 2048, "0198010413a023a24e350100422002000021020c00000001060c");
	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "") != 0)
	{
		printf("Test end_to_end_setupcomplete_to_identity_request second fragment failed\n");
		error = 1;
	}

	// Third fragment should trigger a status report due to poll bit.
	len = hex_to_bytes(bytes, 2048, "3d0100b00200000000001a000014012022069e314de200000000");
	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "210d21021fa000000800183aa80800000000000c") != 0)
	{
		printf("Test end_to_end_setupcomplete_to_identity_request third fragment failed\n");
		error = 1;
	}

	return error;
}

//TEST_METHOD(ping_checksum_isvalid)
int ping_checksum_isvalid()
{
	// From https://support.microsoft.com/en-gb/kb/217014
	unsigned char bytes[2048];
	int len;
	int error = 0;

	len = ping(bytes, 2048, 0x0c4a65be, 0xd088fb6f);
	if (0x7b != (int)bytes[10] || 0xb4 != (int)bytes[11])
	{
		printf("Test ping_checksum_isvalid failed\n");
		error = 1;
	}

	return error;
}

int end_to_end_identity_response_works()
{
	unsigned char bytes[2048], tx_bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, tx_len;

	reset_state();

	len = hex_to_bytes(bytes, 2048, "3D21151F00A00301480160EAC101220202064A8ED30000000000");
	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "212e21021fa000000801203a9002aaaaaaaaaaaaaaaaaaaaaaaaaaaaaab082b28a829a92e40002aaa2bab28a8e9a90000000000010") != 0)
	{
		printf("Test end_to_end_identity_response_works failed\n");
		error = 1;
	}

	return error;
}

int mac_decode_survives_padding_at_start()
{
	unsigned char bytes[2048];
	int len;
	int error = 0;

	sdu out[64];
	int len_out;

	len = hex_to_bytes(bytes, 2048, "3f1d00");
	len_out = mac_decode(out, 64, bytes, len);
	if (len_out != 0)
	{
		printf("Test mac_decode_survives_padding_at_start 3f1d00 failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "3f3f1d00");
	len_out = mac_decode(out, 64, bytes, len);
	if (len_out != 0)
	{
		printf("Test mac_decode_survives_padding_at_start 3f3f1d00 failed\n");
		error = 1;
	}

	return error;
}

int end_to_end_increments_rlc_sn()
{
	unsigned char bytes[2048], tx_bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, tx_len;

	reset_state();

	len = hex_to_bytes(bytes, 2048, "3D21151F00A00301480160EAC101220202064A8ED30000000000");
	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "212e21021fa000000801203a9002aaaaaaaaaaaaaaaaaaaaaaaaaaaaaab082b28a829a92e40002aaa2bab28a8e9a90000000000010") != 0)
	{
		printf("Test end_to_end_increments_rlc_sn failed\n");
		error = 1;
	}

	// RLC header A000 should have changed to A000
	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "212e21021fa001000801203a9002aaaaaaaaaaaaaaaaaaaaaaaaaaaaaab082b28a829a92e40002aaa2bab28a8e9a90000000000010") != 0)
	{
		printf("Test end_to_end_increments_rlc_sn failed\n");
		error = 1;
	}

	reset_state();

	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "212e21021fa000000801203a9002aaaaaaaaaaaaaaaaaaaaaaaaaaaaaab082b28a829a92e40002aaa2bab28a8e9a90000000000010") != 0)
	{
		printf("Test end_to_end_increments_rlc_sn failed\n");
		error = 1;
	}

	return error;
}

int rlc_decode_handles_status_report()
{
	unsigned char bytes[2048];
	unsigned char out[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, len_out;
	unsigned short rlc_sn = 0;
	unsigned char rlc_poll_flag = 0;
	unsigned char rlc_sdu_buffer[MAX_MAC_MSG_SIZE];
	unsigned char rlc_sdu_buffer_index;


	len = hex_to_bytes(bytes, 2048, "002A0400");
	len_out = rlc_decode(out, 2048, 1, bytes, len, &rlc_sn, &rlc_poll_flag, rlc_sdu_buffer, &rlc_sdu_buffer_index);
	if (len_out != 0)
	{
		printf("Test rlc_decode_handles_status_report: \"002A0400\" failed\n");
		error = 1;
	}

	return error;
}

int detach_request_fragment_is_handled_correctly()
{
	unsigned char bytes[2048], tx_bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int error = 0;
	int len, tx_len;

	reset_state();

	len = hex_to_bytes(bytes, 2048, "3d0201a805034802a4e00000000080e8a0617ec01e2200000006");
	tx_len = end_to_end(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "22021f0018") != 0)
	{
		printf("Test detach_request_fragment_is_handled_correctly failed\n");
		error = 1;
	}

	return error;
}

int checksum_is_calculated_correctly()
{
	unsigned int error = 0;
	unsigned char bytes[2048];
	int len = hex_to_bytes(bytes, 2048, "4500003CE10C0000200100000C4A65BED088FB6F0800965B0800AF006162636465666768696A6B6C6D6E6F7071727374757677616263646566676869");
	unsigned short chk = checksum(bytes, len);
	if (chk != 0x7BB4)
	{
		printf("Test checksum_is_calculated_correctly failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "080000000800AF006162636465666768696A6B6C6D6E6F7071727374757677616263646566676869");
	chk = checksum(bytes, len);
	if (chk != 0x965B)
	{
		printf("Test checksum_is_calculated_correctly failed\n");
		error = 1;
	}

	len = hex_to_bytes(bytes, 2048, "08000000000100266162636465666768696A6B6C6D6E6F7071727374757677616263646566676869");
	chk = checksum(bytes, len);
	if (chk != 0x4D35)
	{
		printf("Test checksum_is_calculated_correctly failed\n");
		error = 1;
	}

	return error;
}

int icmp_handles_ping_request()
{
	unsigned int error = 0;
	unsigned char bytes[2048], tx_bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	int len = hex_to_bytes(bytes, 2048, "08004D35000100266162636465666768696A6B6C6D6E6F7071727374757677616263646566676869");
	int tx_len = icmp(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "00005535000100266162636465666768696a6b6c6d6e6f7071727374757677616263646566676869") != 0)
	{
		printf("Test icmp_handles_ping_request failed\n");
		error = 1;
	}
	return error;
}

int ip_handles_ping_request()
{
	unsigned int error = 0;
	unsigned char bytes[2048], tx_bytes[2048];
	char str[1000];
	const char *cstr = (char *)str;
	
	// https://wiki.wireshark.org/Internet_Control_Message_Protocol example .pcap file
	int len = hex_to_bytes(bytes, 2048, "4500003cc63e0000ff01f2d7c0a80059c0a800010800425c020009006162636465666768696a6b6c6d6e6f7071727374757677616263646566676869");
	int tx_len = ip(bytes, len, tx_bytes, 2048);
	bytes_to_hex(str, 1000, tx_bytes, tx_len);
	if (strcmp(cstr, "4500003cc63e0000ff0173d7c0a80001c0a8005900004a5c020009006162636465666768696a6b6c6d6e6f7071727374757677616263646566676869") != 0)
	{
		printf("Test ip_handles_ping_request failed\n");
		error = 1;
	}
	return error;
}

int test_mac_msg_parsing()
{
	int error = 0;

	error = bytes_to_hex_works() || error;
	error = hex_to_bytes_works() || error;
	error = mac_decode_strips_header() || error;
	error = mac_encode_adds_header() || error;
	error = rlc_decode_strips_header() || error;
	error = rlc_encode_adds_header() || error;
	error = pdcp_decode_strips_header() || error;
	error = pdcp_encode_adds_header() || error;
	error = rrc_handles_rrcconnectionrequest() || error;
	error = end_to_end_request_to_setup() || error;
	error = ping_checksum_isvalid() || error;
	error = end_to_end_setupcomplete_to_identity_request() || error;
	error = end_to_end_identity_response_works() || error;
	error = mac_decode_survives_padding_at_start() || error;
	error = end_to_end_increments_rlc_sn() || error;
	error = rlc_decode_handles_status_report() || error;
	error = detach_request_fragment_is_handled_correctly() || error;
	error = checksum_is_calculated_correctly() || error;
	error = icmp_handles_ping_request() || error;
	error = ip_handles_ping_request() || error;

	return !error;
}