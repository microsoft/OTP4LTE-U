#ifndef WIFI_PACKET_H_
#define WIFI_PACKET_H_

#include "headers.h"
#include <pcap/pcap.h>
#define SNAP_LEN 1518

//AP
#define ASSOCIATION_RESPONSE 0x01
#define REASSOCIATION_RESPONSE 0x03
#define PROBE_RESPONSE 0x05
#define BEACON 0x08
#define ATIM 0x09
#define DISASSOCIATION 0x0A


//Client
#define ASSOCIATION_REQUEST 0x00
#define REASSOCIATION_REQUEST 0x02
#define PROBE_REQUEST 0x04
#define AUTHENTICATION 0x0B
//Both

#define DEAUTHENTICATION 0x0C
#define ACTION 0x0D

#define BLOCKREQ 0x18
#define BLOCKACK 0x19

#define RTS 0x1B
#define CTS 0x1C
#define ACK 0x1D
#define DATA 0x20
#define QOSDATA 0x28
/*
header length:18,rate:12
header length:21,rate:0
header length:32,rate:0
header length:14,rate:0
*/

const int kPreambleDuration = 20; //us for short preamble

class WiFiPacket {
private:
  uint64_t time_;
  uint16_t header_len_;
  uint16_t packet_len_;
  uint16_t payload_len_;
  uint16_t freq_;
  int16_t rssi_;
  double rate_;
  uint8_t type_;
  string tmac_;
  string rmac_;
  uint16_t duration_;
  bool tods_;
  bool fromds_;
  bool correct_;
  bool retx_;
  bool longPreamble_;
  uint8_t bandwidth_;
  //1 for 40MHz!, 0 for 20Mhz? 
  uint8_t guard_; 
  //1 for short 0.4us, 0 for long 0.8us
public:
  WiFiPacket();
  ~WiFiPacket();
  double preambleDuration() const {return longPreamble_==true?kPreambleDuration*2:kPreambleDuration;}
  int frequency() const {return freq_;}
  uint64_t time() const {return time_;}
  int headerLen() const {return header_len_;}
  int packetLen() const {return packet_len_;}
  int payloadLen() const {return payload_len_;}
  int rssi() const {return rssi_;}
  int type() const {return type_;}
  double rate() const {return rate_;}
  string tmac() const {return tmac_;}
  string rmac() const {return rmac_;}
  int duration() const {return duration_;}
  int bandwidth() const {return bandwidth_==1?40:20;}
  bool tods() const {return tods_;}
  bool fromds() const {return fromds_;}
  bool correct() const {return correct_;}
  bool retx() const {return retx_;}
  void parsePacket(uint64_t time, uint16_t len, const u_char* packet);
  string metadata();
  
  void setTime(uint64_t t) {time_ = t;}
  void parseMetadata(string meta);
};

//null function (very rare), with mac 24 bytes
//instead of 26 bytes
struct wifi_radiotap_header_14 {
    u_int8_t it_version;
    u_int8_t it_vpad;
    u_int16_t it_len;
    u_int32_t it_present;
    u_int8_t it_pad[3];
    u_int8_t it_mcs;
    u_int8_t it_known_info;
    //unsigned int it_known_info_pad:5;
    //unsigned int it_known_info_guard:1;
    //unsigned int it_known_info_bandwidth:2;
    u_int8_t it_mcs_index;
};

//802.11n?
struct wifi_radiotap_header_21 {
    u_int8_t it_version;
    u_int8_t it_vpad;
    u_int16_t it_len;
    u_int32_t it_present;
    u_int8_t it_flags;
    u_int8_t it_fpad;
    u_int16_t it_frequency;
    u_int16_t it_channel_type;
    u_int8_t it_ssi;
    u_int8_t it_antenna;
    u_int16_t it_rx; 
    u_int8_t it_mcs;
    u_int8_t it_known_info;
    //int it_known_info_pad:5;
    //int it_known_info_guard:1;
    //int it_known_info_bandwidth:2; 
    u_int8_t it_mcs_index;
};

//802.11g?
struct wifi_radiotap_header_18 {
    u_int8_t it_version;
    u_int8_t it_vpad;
    u_int16_t it_len;
    u_int32_t it_present;
    u_int8_t it_flags;
    u_int8_t it_rate;
    u_int16_t it_frequency;
    u_int16_t it_channel_type;
    u_int8_t it_ssi;
    u_int8_t it_antenna;
    u_int16_t it_rx;
};

struct wifi_radiotap_header_13 {
    u_int8_t it_version;
    u_int8_t it_vpad;
    u_int16_t it_len;
    u_int32_t it_present;
    u_int8_t it_rate;
    u_int8_t it_pad[4];
};


//length: 26 bytes
//only 10 bytes for acknowledgement
struct wifi_mac {
    //struct frame_control fc; 
    unsigned int protoVer:2; // protocol version
    unsigned int type:2; //frame type field (Management,Control,Data)
    unsigned int subtype:4; // frame subtype    
    unsigned int toDS:1; // frame coming from Distribution system 
    unsigned int fromDS:1; //frame coming from Distribution system 
    unsigned int moreFrag:1; // More fragments?
    unsigned int retry:1; //was this frame retransmitted    
    unsigned int powMgt:1; //Power Management
    unsigned int moreDate:1; //More Date
    unsigned int protectedData:1; //Protected Data
    unsigned int order:1; //Order

    u_int16_t wi_duration; //16
    u_int8_t wi_receiver[6]; //receiver
    u_int8_t wi_transmitter[6]; //trasmitter
    u_int8_t wi_bssid[6];   
    u_int16_t wi_sequenceControl;
    u_int16_t wi_qoscontrol;
    //u_int8_t wi_parameters[8];
    //unsigned int frameBody[23124];
};

const double mcstable[32][5] = {
{0,6.5,7.2,13.5,15},
{1,13,14.4,27,30},
{2,19.5,21.7,40.5,45},
{3,26,28.9,54,60},
{4,39,43.3,81,90},
{5,52,57.8,108,120},
{6,58.5,65,121.5,135},
{7,65,72.2,135,150},
{8,13,14.4,27,30},
{9,26,28.9,54,60},
{10,39,43.3,81,90},
{11,52,57.8,108,120},
{12,78,86.7,162,180},
{13,104,115.6,216,240},
{14,117,130,243,270},
{15,130,144.4,270,300},
{16,19.5,21.7,40.5,45},
{17,39,43.3,81,90},
{18,58.5,65,121.5,135},
{19,78,86.7,162,180},
{20,117,130,243,270},
{21,156,173.3,324,360},
{22,175.5,195,364.5,405},
{23,195,216.7,405,450},
{24,26,28.8,54,60},
{25,52,57.6,108,120},
{26,78,86.8,162,180},
{27,104,115.6,216,240},
{28,156,173.2,324,360},
{29,208,231.2,432,480},
{30,234,260,486,540},
{31,260,288.8,540,600},
};

double static lookupMCS(int index, int bandwidth, int guard)
{ 
  //cout<<"lookup:"<<index<<","<<bandwidth<<","<<guard<<endl;
  return mcstable[index][1 + bandwidth * 2 + guard];
}

const uint32_t crctable[] = {
   0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
   0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
   0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
   0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
   0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
   0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
   0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
   0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
   0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
   0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
   0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
   0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
   0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
   0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
   0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
   0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
   0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
   0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
   0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
   0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
   0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
   0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
   0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
   0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
   0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
   0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
   0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
   0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
   0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
   0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
   0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
   0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

uint32_t static crc32(uint32_t bytes_sz, const uint8_t *bytes)
{
   uint32_t crc = ~0;
   uint32_t i;
   for(i = 0; i < bytes_sz; ++i) {
      crc = crctable[(crc ^ bytes[i]) & 0xff] ^ (crc >> 8);
   }
   return ~crc;
}

#endif
