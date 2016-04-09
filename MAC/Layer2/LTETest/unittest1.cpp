#include "stdafx.h"
#include "CppUnitTest.h"

namespace Microsoft
{
	namespace VisualStudio
	{
		namespace CppUnitTestFramework
		{
			template<> static std::wstring ToString<std::vector<unsigned char>>(const std::vector<unsigned char>& t)
			{ // TODO: std::copy(t.begin(), t.end()--, ",") << *t.end()
				std::wstringstream _s;
				for (auto tt : t)
					_s << tt << ' ';
				return _s.str();
			}
		}
	}
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LTETest
{
	std::string bytes_to_hex(std::vector<unsigned char> bytes)
	{
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		for (unsigned b : bytes)
			ss << std::setw(2) << b;
		return ss.str();
	}

	std::vector<unsigned char> hex_to_bytes(std::string hex)
	{
		const char* input = hex.c_str();
		std::vector<unsigned char> bytes;
		for (size_t i = 0; i < hex.length(); i += 2)
		{
			unsigned int byte = 0;
			sscanf_s(input + i, "%2x", &byte);
			bytes.push_back(byte);
		}
		return bytes;
	}

	struct sdu { unsigned char lcid; std::vector<unsigned char> bytes; };

	// MAC UL-SCH decode
	std::vector<sdu> mac_decode(std::vector<unsigned char> bytes)
	{
		std::map<unsigned char, unsigned char> lcid_length{ { 26, 1 }, { 27, 2 }, { 28, 1 }, { 29, 1 }, { 30, 3 } };

		struct range { unsigned char lcid; unsigned int start; unsigned int length; };
		std::vector<range> headers;

		size_t i = 0, start = 0;
		unsigned char e = 1; // Extension field; 0 => no more headers
		while (i < bytes.size() && e == 1)
		{
			unsigned char byte = bytes[i++];
			unsigned char lcid = byte & 0x1f;
			e = (byte >> 5) & 1;

			unsigned int length = lcid_length[lcid];
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
					length = bytes.size() - start - i; // last subheader, takes all remaining payload bytes
			}


			if (lcid <= 10) // keep SDUs, ignore Control Elements
				headers.push_back({ lcid, start, length });
			start += length;
		}

		// At this point, i is the index of the first payload byte (if there is one).
		std::vector<sdu> sdus;
		for (auto h : headers)
		{
			auto start = bytes.begin() + i + h.start;
			sdus.push_back({ h.lcid, std::vector<unsigned char>(start, start + h.length) });
		}
		return sdus;
	}

	// MAC DL-SCH encode
	std::vector<unsigned char> mac_encode(std::vector<sdu> sdus)
	{
		std::vector<unsigned char> bytes;

		for (size_t i = 0; i < sdus.size(); i++)
		{
			auto s = sdus[i];
			unsigned char e = (i < sdus.size() - 1);
			unsigned char h = (e << 5) | (s.lcid & 0x1f);
			bytes.push_back(h);
			if (e == 1 && s.lcid <= 10)
			{
				size_t length = s.bytes.size();
				unsigned char f = (length > 127);
				if (f == 1)
				{
					h = (f << 7) | ((length >> 8) & 0x7f);
					bytes.push_back(h);
					h = length & 0xff;
					bytes.push_back(h);
				}
				else
				{
					h = (f << 7) | (length & 0x7f);
					bytes.push_back(h);
				}
			}
		}

		// Note: I have not bothered with 1f padding; I suspect it is unnecessary.

		for (auto s : sdus)
			for (auto b : s.bytes)
				bytes.push_back(b);

		return bytes;
	}

	// RLC UL-SCH decode
	std::vector<unsigned char> rlc_decode(unsigned char lcid, std::vector<unsigned char> bytes)
	{
		size_t i = 0;

		if (lcid == 0) // Transparent Mode
			;
		else if (lcid == 1 || lcid == 2) // Acknowledged Mode
		{
			unsigned char byte = bytes[i++];
			unsigned char dc = (byte >> 7) & 1;
			if (dc == 1)
			{
				unsigned char rf = (byte >> 6) & 1;
				unsigned char p = (byte >> 5) & 1;
				unsigned char fi = (byte >> 3) & 3;
				unsigned char e = (byte >> 2) & 1;
				unsigned int sn = byte & 3;

				byte = bytes[i++];
				sn = (sn << 8) + byte;

				assert(fi == 0); // TODO - fragment reassembly
				assert(e == 0); // TODO
			}
			else
			{
				// STATUS - ignored
				unsigned int ack_sn = (byte & 0xf);

				byte = bytes[i++];
				ack_sn = (ack_sn << 6) + ((byte >> 2) & 0x3f);
				unsigned char e1 = (byte >> 1) & 1;
				assert(e1 == 0); // TODO
			}
		}
		else // Unacknowledged Mode
		{
			unsigned char byte = bytes[i++];
			unsigned char fi = (byte >> 6) & 3;
			unsigned char e = (byte >> 5) & 1;
			unsigned char sn = byte & 0x1f;

			assert(fi == 0); // TODO - fragment reassembly
			assert(e == 0); // TODO
		}

		return std::vector<unsigned char>(bytes.begin() + i, bytes.end());
	}

	// RLC DL-SCH encode
	std::vector<unsigned char> rlc_encode(unsigned char lcid, unsigned int& sn, std::vector<unsigned char> bytes)
	{
		std::vector<unsigned char> mac_bytes;
		
		if (lcid == 0 || lcid >= 28) // Transparent Mode
			;
		else if (lcid == 1 || lcid == 2) // Acknowledged Mode
		{
			unsigned char h = (sn >> 8) & 3; // other fields all zero
			mac_bytes.push_back(h);
			h = sn & 0xff;
			mac_bytes.push_back(h);
			sn++;
		}
		else // Unacknowledged Mode
		{
			unsigned char h = (sn & 0x1f); // other fields all zero
			mac_bytes.push_back(h);
			sn++;
		}

		for (auto b : bytes)
			mac_bytes.push_back(b);

		return mac_bytes;
	}

	// PDCP UL-SCH decode
	std::vector<unsigned char> pdcp_decode(unsigned char lcid, std::vector<unsigned char> bytes)
	{
		size_t i = 0, j = 0;

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
				i = bytes.size(); // ignore control packet
			}
		}

		return std::vector<unsigned char>(bytes.begin() + i, bytes.end() - j);
	}

	// PDCP DL-SCH encode
	std::vector<unsigned char> pdcp_encode(unsigned char lcid, unsigned int& sn, std::vector<unsigned char> bytes)
	{
		std::vector<unsigned char> rlc_bytes;

		if (lcid == 0 || lcid >= 28) // Transparent Mode
			for (auto b : bytes)
				rlc_bytes.push_back(b);
		else if (lcid == 1 || lcid == 2) // Control plane
		{
			unsigned char h = (sn & 0x1f); // other fields all zero
			rlc_bytes.push_back(h);
			sn++;
			for (auto b : bytes)
				rlc_bytes.push_back(b);
			for (int i = 0; i < 4; i++)
				rlc_bytes.push_back(0); // MAC-I
		}
		else // User plane
		{
			unsigned char h = (sn & 0x7f);
			rlc_bytes.push_back(h);
			sn++;
			for (auto b : bytes)
				rlc_bytes.push_back(b);
		}

		return rlc_bytes;
	}

	std::vector<unsigned char> ping(unsigned int source_ip, unsigned int dest_ip)
	{
		//60 bytes. IP checksum: sum shorts, add carries, flip bits
		//IP:        45 00 00 3C E1 0C 00 00 20 01 
		//*checksum:  7B B4
		//*source IP: 0C 4A 65 BE
		//*dest IP:   D0 88 FB 6F
		//ICMP: 08 00 96 5B 08 00 AF 00
		//payload: 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 74 75 76 77 61 62 63 64 65 66 67 68 69 a..wa..i

		// IP header
		std::vector<unsigned char> bytes;
		for (auto b : hex_to_bytes("4500003ce10c00002001")) bytes.push_back(b); // IP header
		for (auto b : hex_to_bytes("0000")) bytes.push_back(b); // checksum - to be overwritten
		bytes.push_back(source_ip >> 24); bytes.push_back(source_ip >> 16); bytes.push_back(source_ip >> 8); bytes.push_back(source_ip);
		bytes.push_back(dest_ip >> 24); bytes.push_back(dest_ip >> 16); bytes.push_back(dest_ip >> 8); bytes.push_back(dest_ip);

		// IP header checksum
		std::vector<unsigned short> shorts;
		for (size_t i = 0; i < bytes.size(); i += 2)
		{
			unsigned short s = (bytes[i] << 8) | bytes[i + 1];
			shorts.push_back(s);
		}
		unsigned int checksum = 0;
		for (auto s : shorts) checksum += s;
		while (checksum >> 16) checksum = (checksum & 0xffff) + (checksum >> 16);
		checksum = ~checksum;
		bytes[10] = checksum >> 8;
		bytes[11] = checksum & 0xff;

		// ICMP packet
		for (auto b : hex_to_bytes("0800965b0800af00")) bytes.push_back(b); // ICMP header
		for (auto b : hex_to_bytes("6162636465666768696a6b6c6d6e6f7071727374757677616263646566676869")) bytes.push_back(b);
		return bytes;
	}

	// DL response to UL RRC
	std::vector<sdu> rrc(unsigned char lcid, std::vector<unsigned char> bytes)
	{
		std::vector<sdu> sdus;
		if (lcid == 0) // UL-CCCH-Message
		{
			assert(bytes[0] >> 6 == 1); // RRCConnectionRequest
			sdus.push_back({ 28, bytes }); // Contention Resolution ID control element
			auto msg = hex_to_bytes("600000"); // RRCConnectionSetup
			sdus.push_back({ 0, msg });
		}
		else if (lcid == 1 || lcid == 2) // UL-DCCH-Message
		{
			unsigned char c1 = (bytes[0] >> 3);
			if (c1 == 2) // RRCConnectionReconfigurationComplete
			{
				// We should be ok to send IP traffic on LCID 3 at this point. Try a ping.
				auto msg = ping(0xc0a80101, 0xc0a80102);
				sdus.push_back({ 3, msg });
			}
			else if (c1 == 4) // RRCConnectionSetupComplete
			{
				// RRCConnectionReconfiguration
				std::vector<unsigned char> msg = hex_to_bytes("200600a03a1000000000581006080000280e054008100001007800bc2000");
				sdus.push_back({ 1, msg });
			}
		}
		else // IP traffic
		{
			// TODO: should receive ping response here.
		}

		return sdus;
	}

	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(bytes_to_hex_works)
		{
			auto test = [](std::string e, std::vector<unsigned char> a) { Assert::AreEqual(e, bytes_to_hex(a)); };
			test("00", { 0 });
			test("01", { 1 });
			test("0f", { 15 });
			test("ff", { 255 });
			test("0000", { 0, 0 });
			test("0100", { 1, 0 });
		}

		TEST_METHOD(hex_to_bytes_works)
		{
			auto test = [](std::vector<unsigned char> e, std::string a) { Assert::AreEqual(e, hex_to_bytes(a)); };
			test({}, "");
			test({ 0 }, "00");
			test({ 1 }, "01");
			test({ 15 }, "0f");
			test({ 255 }, "ff");
			test({ 0, 0 }, "0000");
			test({ 1, 0 }, "0100");
		}

		TEST_METHOD(mac_decode_strips_header)
		{
			auto ans = mac_decode({});
			Assert::AreEqual(0, (int)ans.size());

			ans = mac_decode(hex_to_bytes("1f"));
			Assert::AreEqual(0, (int)ans.size());

			// Captured from dongle.
			ans = mac_decode(hex_to_bytes("20061f500a5024b3a6"));
			Assert::AreEqual(1, (int)ans.size());
			Assert::AreEqual(0, (int)ans[0].lcid);
			Assert::AreEqual("500a5024b3a6", bytes_to_hex(ans[0].bytes).c_str());

			// From http://www.sharetechnote.com/html/MAC_LTE.html
			// Had to change length from 27 to 2f to make sense, think there was an error on his page
			ans = mac_decode(hex_to_bytes("3d212f1f00a000002020000000260741010bf60000000000000000000002e06000050201d011d0190000025200000000005c080200000000"));
			Assert::AreEqual(1, (int)ans.size());
			Assert::AreEqual(1, (int)ans[0].lcid);
			Assert::AreEqual("a000002020000000260741010bf60000000000000000000002e06000050201d011d0190000025200000000005c0802", bytes_to_hex(ans[0].bytes).c_str());

			ans = mac_decode(hex_to_bytes("3d210922111f00a00404100040ebc014a000004800e0e860006a401840000000000000000000000000"));
			Assert::AreEqual(2, (int)ans.size());
			Assert::AreEqual(1, (int)ans[0].lcid);
			Assert::AreEqual("a00404100040ebc014", bytes_to_hex(ans[0].bytes).c_str());
			Assert::AreEqual(2, (int)ans[1].lcid);
			Assert::AreEqual("a000004800e0e860006a40184000000000", bytes_to_hex(ans[1].bytes).c_str());

			ans = mac_decode(hex_to_bytes("031804509ac2bc25c2f0bb0ba0b682f09a50970970bc26942f0ab0ba0bc2f0bc2f093a092709a50bc26942da0970ae824e82da0ab0b70b50bc2ba09a4c2d42750ab0bc2ec24d42f0bc2f0970bc26942f0970bc25c2bc2a4c2f0ab0bc2f0b682f0ab0ab09a50bc2da0bc2f0bc2ec2d42f0bc2ac2ac247004414f0b682f0bc25c2ba0ab0bc2f0bc25c2da0bc2f0ba0bc2f0b682da0bc269424e825c2f0bc2f0ae825c2f0bc2da0b50bc2f0bc2f0bc25c2d42f09d30bc2ec2d42f0bc2f0bc2ec2d425c26942f0bc2f0bc2f09a50bc2f0ab0bc2ac2f0bc2f0bc2f0ab09a50b682f0bc2"));
			Assert::AreEqual(1, (int)ans.size());
			Assert::AreEqual(3, (int)ans[0].lcid);
			Assert::AreEqual("1804509ac2bc25c2f0bb0ba0b682f09a50970970bc26942f0ab0ba0bc2f0bc2f093a092709a50bc26942da0970ae824e82da0ab0b70b50bc2ba09a4c2d42750ab0bc2ec24d42f0bc2f0970bc26942f0970bc25c2bc2a4c2f0ab0bc2f0b682f0ab0ab09a50bc2da0bc2f0bc2ec2d42f0bc2ac2ac247004414f0b682f0bc25c2ba0ab0bc2f0bc25c2da0bc2f0ba0bc2f0b682da0bc269424e825c2f0bc2f0ae825c2f0bc2da0b50bc2f0bc2f0bc25c2d42f09d30bc2ec2d42f0bc2f0bc2ec2d425c26942f0bc2f0bc2f09a50bc2f0ab0bc2ac2f0bc2f0bc2f0ab09a50b682f0bc2", bytes_to_hex(ans[0].bytes).c_str());
		}

		TEST_METHOD(mac_encode_adds_header)
		{
			auto ans = mac_encode({});
			Assert::AreEqual(0, (int)ans.size());

			ans = mac_encode({ { 0, {} } });
			Assert::AreEqual("00", bytes_to_hex(ans).c_str());

			ans = mac_encode({ { 0, { 0 } } });
			Assert::AreEqual("0000", bytes_to_hex(ans).c_str());

			ans = mac_encode({ { 0, { 1 } } });
			Assert::AreEqual("0001", bytes_to_hex(ans).c_str());

			ans = mac_encode({ { 1, { 0 } } });
			Assert::AreEqual("0100", bytes_to_hex(ans).c_str());

			ans = mac_encode({ { 0, { 0 } }, { 1, { 0 } } });
			Assert::AreEqual("2001010000", bytes_to_hex(ans).c_str());

			ans = mac_encode({ { 1, { 0 } }, { 0, { 0 } } });
			Assert::AreEqual("2101000000", bytes_to_hex(ans).c_str());

			ans = mac_encode({ { 0, hex_to_bytes("500a5024b3a6") } });
			Assert::AreEqual("00500a5024b3a6", bytes_to_hex(ans).c_str());

			ans = mac_encode({ { 1, hex_to_bytes("a00404100040ebc014") }, { 2, hex_to_bytes("a000004800e0e860006a40184000000000") } });
			Assert::AreEqual("210902a00404100040ebc014a000004800e0e860006a40184000000000", bytes_to_hex(ans).c_str());
		}

		TEST_METHOD(rlc_decode_strips_header)
		{
			auto ans = rlc_decode(0, {});
			Assert::AreEqual(0, (int)ans.size());

			ans = rlc_decode(0, hex_to_bytes("500a5024b3a6"));
			Assert::AreEqual("500a5024b3a6", bytes_to_hex(ans).c_str());

			// From http://www.sharetechnote.com/html/RLC_LTE.html
			ans = rlc_decode(1, hex_to_bytes("0004")); // STATUS
			Assert::AreEqual("", bytes_to_hex(ans).c_str());

			ans = rlc_decode(1, hex_to_bytes("a00101480160ea611479e5cbce4d6ad68000000000"));
			Assert::AreEqual("01480160ea611479e5cbce4d6ad68000000000", bytes_to_hex(ans).c_str());

			// TODO: two RLC segments
			//ans = rlc_decode(3, hex_to_bytes("1ce1068fcc4823e990da18d22b1ba3d95c72e8d771a951a982781780dc9b1876179918487f7722b98e79daa9585ac86e1c93f91d64c45856460cb5a30c5eb324498ff051fcd7a0f629a35173c2319ef84b0f0734b106a8e179e0e0ddc7c8fc235208d8f448ed9a7010efdd1c8054450002500e620000801195c90101c9370101c93804e704d2023c9b6247004411e6f0b01c8033a52c12c2c8351692616812070d81ccc729c921a1b9946c2522e328781a6ae35a501b138333c657114d5b195508312675fa4b42fb84ff2b1be222b39c670720ac24070d896b4d072457bc94cf"));
			//Assert::AreEqual("01480160ea611479e5cbce4d6ad68000000000", bytes_to_hex(ans).c_str());
		}

		TEST_METHOD(rlc_encode_adds_header)
		{
			unsigned int sn = 0;
			auto ans = rlc_encode(0, sn, {});
			Assert::AreEqual(0u, sn);
			Assert::AreEqual("", bytes_to_hex(ans).c_str());

			ans = rlc_encode(0, sn = 0, hex_to_bytes("500a5024b3a6"));
			Assert::AreEqual(0u, sn);
			Assert::AreEqual("500a5024b3a6", bytes_to_hex(ans).c_str());

			ans = rlc_encode(1, sn = 0, {});
			Assert::AreEqual(1u, sn);
			Assert::AreEqual("0000", bytes_to_hex(ans).c_str());

			ans = rlc_encode(2, sn = 0, {});
			Assert::AreEqual(1u, sn);
			Assert::AreEqual("0000", bytes_to_hex(ans).c_str());

			ans = rlc_encode(3, sn = 0, {});
			Assert::AreEqual(1u, sn);
			Assert::AreEqual("00", bytes_to_hex(ans).c_str());
		}

		TEST_METHOD(pdcp_decode_strips_header)
		{
			auto ans = pdcp_decode(0, {});
			Assert::AreEqual(0, (int)ans.size());

			ans = pdcp_decode(0, hex_to_bytes("500a5024b3a6"));
			Assert::AreEqual("500a5024b3a6", bytes_to_hex(ans).c_str());

			ans = pdcp_decode(1, hex_to_bytes("000000000000"));
			Assert::AreEqual("00", bytes_to_hex(ans).c_str());

			ans = pdcp_decode(2, hex_to_bytes("000100000000"));
			Assert::AreEqual("01", bytes_to_hex(ans).c_str());

			ans = pdcp_decode(3, hex_to_bytes("0000")); // ROHC feedback
			Assert::AreEqual("", bytes_to_hex(ans).c_str());

			ans = pdcp_decode(3, hex_to_bytes("010001020304")); // PDCP status report
			Assert::AreEqual("", bytes_to_hex(ans).c_str());

			ans = pdcp_decode(3, hex_to_bytes("8000"));
			Assert::AreEqual("00", bytes_to_hex(ans).c_str());
		}

		TEST_METHOD(pdcp_encode_adds_header)
		{
			unsigned int sn = 0;
			auto ans = pdcp_encode(0, sn, {});
			Assert::AreEqual(0u, sn);
			Assert::AreEqual("", bytes_to_hex(ans).c_str());

			ans = pdcp_encode(0, sn = 0, hex_to_bytes("500a5024b3a6"));
			Assert::AreEqual(0u, sn);
			Assert::AreEqual("500a5024b3a6", bytes_to_hex(ans).c_str());

			ans = pdcp_encode(1, sn = 0, {});
			Assert::AreEqual(1u, sn);
			Assert::AreEqual("0000000000", bytes_to_hex(ans).c_str());

			ans = pdcp_encode(2, sn = 0, {255});
			Assert::AreEqual(1u, sn);
			Assert::AreEqual("00ff00000000", bytes_to_hex(ans).c_str());

			ans = pdcp_encode(3, sn = 0, {});
			Assert::AreEqual(1u, sn);
			Assert::AreEqual("00", bytes_to_hex(ans).c_str());
		}

		TEST_METHOD(rrc_handles_rrcconnectionrequest)
		{
			auto ans = rrc(0, hex_to_bytes("500a5024b3a6"));
			Assert::AreEqual(2, (int)ans.size());
			Assert::AreEqual(28, (int)ans[0].lcid);
			Assert::AreEqual("500a5024b3a6", bytes_to_hex(ans[0].bytes).c_str());
			Assert::AreEqual(0, (int)ans[1].lcid);
			Assert::AreEqual("600000", bytes_to_hex(ans[1].bytes).c_str());
		}

		TEST_METHOD(end_to_end_request_to_setup)
		{
			auto mac = mac_decode(hex_to_bytes("20061f500a5024b3a6"));
			auto sdus = rrc(mac[0].lcid, mac[0].bytes);
			auto ans = mac_encode(sdus);

			Assert::AreEqual("3c00500a5024b3a6600000", bytes_to_hex(ans).c_str());
		}

		TEST_METHOD(ping_checksum_isvalid)
		{
			// From https://support.microsoft.com/en-gb/kb/217014
			auto ans = ping(0x0c4a65be, 0xd088fb6f);
			Assert::AreEqual(0x7b, (int)ans[10]);
			Assert::AreEqual(0xb4, (int)ans[11]);
		}
	};
}