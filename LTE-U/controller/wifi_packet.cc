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

#include "wifi_packet.h"


WiFiPacket::WiFiPacket() 
{
  time_ = 0;
  header_len_ = 0;
  packet_len_ = 0;
  payload_len_ = 0;
  freq_ = 0;
  rssi_ = 0;
  rate_ = 0;
  type_ = 0;
  tmac_ = "";
  rmac_ = "";
  bandwidth_ = 0;
}


WiFiPacket::~WiFiPacket() 
{

}

string WiFiPacket::metadata()
{
  string meta = to_string(this->type()) + string(",") + 
                  to_string(this->tmac()) + string(",") + 
                  to_string(this->rmac()) + string(",") + 
                  to_string(this->rssi()) + string(",") +
                  to_string(this->rate()) + string(",") +    
                  to_string(this->packetLen()) + string(",") +
                  to_string(this->frequency()) + string(",") +
                  to_string(this->bandwidth()) + string(",") + 
                  to_string(this->tods()) + string(",") + 
                  to_string(this->fromds()) + string(",") +
                  to_string(this->payloadLen());
  return meta;
}

void WiFiPacket::parseMetadata(string meta)
{
  vector<string> elems;
  istringstream iss(meta);
  string s;
  while (getline(iss, s, ',')) {
    elems.push_back(s);
  }
  type_ = atoi(elems[0].c_str());
  tmac_ = elems[1];
  rmac_ = elems[2];
  rssi_ = atoi(elems[3].c_str());
  rate_ = atof(elems[4].c_str());
  packet_len_ = atoi(elems[5].c_str());
  freq_ = atoi(elems[6].c_str());
  bandwidth_ = atoi(elems[7].c_str()) == 40? 1 : 0; 
  tods_ = atoi(elems[8].c_str());
  fromds_ = atoi(elems[9].c_str());
  payload_len_ = atoi(elems[10].c_str());
}

void WiFiPacket::parsePacket(uint64_t time,uint16_t len, const u_char* packet) 
{
  this->time_ = time;
  this->packet_len_ = len;
  this->header_len_ = *(uint16_t*)(packet + 2);  
  int hlen = *(uint16_t*)(packet + 2); 

   
  //bug found here, pkthdr->len is 0 somehow, a hug integer is send to crc32
  if(this->packet_len_ <= hlen + 4) return;
  uint32_t crc = crc32(this->packet_len_ - hlen - 4, packet + hlen);
  uint32_t value = *(uint32_t*)(packet + (this->packet_len_ - 4));
  if(crc != value) {
    //cout<<crc<<" "<<value<<endl;
    this->correct_ = false;
  } else {
    this->correct_ = true;
  }
 struct wifi_radiotap_header_18 *rh18 = (struct wifi_radiotap_header_18 *)packet;  
  struct wifi_radiotap_header_21 *rh21 =(struct wifi_radiotap_header_21 *)packet;  
  struct wifi_radiotap_header_14 *rh14 = (struct wifi_radiotap_header_14 *)packet;
   struct wifi_radiotap_header_13 *rh13 = (struct wifi_radiotap_header_13 *)packet;
 
  switch (this->header_len_) {
    case 13:
     this->rate_ = (double)(rh13->it_rate)/2.0; 
    case 18:
      this->freq_ = rh18->it_frequency;
      this->rssi_ = (int)rh18->it_ssi - 256;
      this->rate_ = (double)(rh18->it_rate)/2.0;
      this->longPreamble_ = rh18->it_flags & 2; 
      break;
    case 21:
    case 32:
      this->freq_ = rh21->it_frequency;
      this->rssi_ = (int)rh21->it_ssi - 256;
      this->bandwidth_ = (int)rh21->it_known_info & 0x01;
      this->guard_ = ((int)rh21->it_known_info & 0x04) >> 2;
  
      this->rate_ = lookupMCS(rh21->it_mcs_index, 
                             this->bandwidth_, this->guard_);
      this->longPreamble_ = rh21->it_flags & 2; 
      break;
    case 14:
      this->bandwidth_ = (int)rh14->it_known_info & 0x01;
      this->guard_ = ((int)rh14->it_known_info & 0x04) >> 2;
//cout<<14<<","<<(uint32_t)rh14->it_mcs_index<<","<<(int)this->bandwidth_<<","<<(int)this->guard_<<endl;  
      this->rate_ = lookupMCS(rh14->it_mcs_index, 
                             this->bandwidth_, this->guard_);
      break;
    default:
      break;
  }
 
  //parse MAC frame 
  struct wifi_mac *wi_mac= (struct wifi_mac *)(packet + this->header_len_);

  this->type_ = ((wi_mac->type << 4) + wi_mac->subtype);
  this->tods_ = wi_mac->toDS;
  this->fromds_ = wi_mac->fromDS;
  this->duration_ = wi_mac->wi_duration;

  if(this->type_ >= DATA) {
    int py = this->packet_len_ - this->header_len_ - sizeof(wifi_mac);  
    this->payload_len_ = py > 0? py : 0;
  }
 
  stringstream buffer;
  for (int i = 0; i < 6; i++) {
    buffer << hex << setfill('0');
    buffer << setw(2)  << static_cast<unsigned>(wi_mac->wi_transmitter[i]); 
  }
  this->tmac_ = buffer.str();
  buffer.str(std::string());
  for (int i = 0; i < 6; i++) {
    buffer << hex << setfill('0');
    buffer << setw(2)  << static_cast<unsigned>(wi_mac->wi_receiver[i]);
  }
  this->rmac_ = buffer.str();
  buffer.str(std::string());

  if(this->type_ == ACK || this->type_ == CTS) {
    //this->tmac_ = "000000000000";
    this->tmac_ = "";
    //assert(0==1);
  }

}


