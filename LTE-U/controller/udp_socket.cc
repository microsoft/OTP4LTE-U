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

#include "udp_socket.h"


UdpSocket::UdpSocket(int32_t pkt_sz/* = kPacketSize*/)
{
  udp_socket_id_ = socket(AF_INET, SOCK_DGRAM, 0);
  assert (udp_socket_id_ != -1);  
  broadcast_server = 0;
  read_packet_size_ = pkt_sz;
}

UdpSocket::~UdpSocket()
{
  close(udp_socket_id_);
}

int32_t UdpSocket::SetNonblocking()
{
  int flags;
  if (-1 == (flags = fcntl(udp_socket_id_, F_GETFL, 0))) {
    flags = 0;
  }
  return fcntl(udp_socket_id_, F_SETFL, flags | O_NONBLOCK);
}

  
void UdpSocket::UdpSocketSetUp(const string& ip, const int32_t& port)
{

  /*UDP server part*/
  udp_port_ = port;

  udp_addr_.sin_family = AF_INET;
  udp_addr_.sin_port = htons(port);
  udp_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
  bzero(&(udp_addr_.sin_zero),8);

  int32_t res_bind = bind(udp_socket_id_, (struct sockaddr *)&udp_addr_, sizeof(struct sockaddr));
  if(-1==res_bind) {
    perror("Error: UdpSocket bind failed in UdpSocketSetUp()");
  }

  int32_t multi_use = 1;
  int32_t check_mu = setsockopt(udp_socket_id_,  SOL_SOCKET,  SO_REUSEADDR,  &multi_use,  sizeof(multi_use));  
  if(-1 == check_mu) {
    perror("Error: UdpSocket set multiple usage failed in UdpSocketSetUp()");
  }
}

void UdpSocket::UdpSocketSetUpBroadcast(const string& ip, const int32_t& port)
{
  broadcast_server = 1;
  udp_broadcast_port_ = port;

  udp_broadcast_addr_.sin_family = AF_INET;
  udp_broadcast_addr_.sin_port = htons(port);
  udp_broadcast_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
  bzero(&(udp_broadcast_addr_.sin_zero),8);

  int32_t broadcastOn = 1;
  int32_t res = setsockopt(udp_socket_id_, SOL_SOCKET, SO_BROADCAST, &broadcastOn, sizeof(broadcastOn));
  if(res == -1) {
		perror("Error: setsockopt call failed in UdpSocketEnableBroadcast()");
  }
}

int32_t UdpSocket::UdpSocketBroadcast(const string& send_data)
{
  assert(1==broadcast_server);
#ifdef DEBUG_UDP_SOCKET
  cout<<"in UdpSocket::SendTo: "<<" dest:"<<string(inet_ntoa(udp_broadcast_addr_.sin_addr))<<" port:"<<ntohs(udp_broadcast_addr_.sin_port)<<" data size:"<<send_data.size()<<endl;
  size_t sz = send_data.size();
  if(sz <= 20) cout<<"data: "<<send_data<<endl;
  else cout<<"data first 10 bytes:"<<send_data.substr(0, 10)<<" data last 10 bytes"<<send_data.substr(sz - 10, 10)<<endl;
#endif
  return sendto(udp_socket_id_, send_data.c_str(), send_data.size(), 0, (struct sockaddr *)&udp_broadcast_addr_, sizeof(struct sockaddr));  
}

int32_t UdpSocket::SendTo(const string& ip, const int32_t& port, const string& send_data)
{
  udp_client_addr_.sin_family = AF_INET;
  udp_client_addr_.sin_port = htons(port);
  udp_client_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
  bzero(&(udp_client_addr_.sin_zero),8);
#ifdef DEBUG_UDP_SOCKET
  cout<<"in UdpSocket::SendTo: "<<" dest:"<<string(inet_ntoa(udp_client_addr_.sin_addr))<<" port:"<<ntohs(udp_client_addr_.sin_port)<<" data size:"<<send_data.size()<<endl;
  size_t sz = send_data.size();
  if(sz <= 20) cout<<"data: "<<send_data<<endl;
  else cout<<"data first 10 bytes:"<<send_data.substr(0, 10)<<" data last 10 bytes"<<send_data.substr(sz - 10, 10)<<endl;
#endif
  return sendto(udp_socket_id_, send_data.c_str(), send_data.size(), 0, (struct sockaddr *)&udp_client_addr_, sizeof(struct sockaddr));  
}

int32_t UdpSocket::ReceiveFrom(string& ip, int32_t& port, string& data)
{
  int32_t addr_len = sizeof(struct sockaddr);
  char recv_data[kPacketSize+1];
  int32_t bytes_read = recvfrom(udp_socket_id_,recv_data, kPacketSize, 0, (struct sockaddr *)&udp_client_addr_, (socklen_t*)&addr_len);
  recv_data[bytes_read] = '\0';
  ip = string(inet_ntoa(udp_client_addr_.sin_addr));
  port = ntohs(udp_client_addr_.sin_port);
  data = string(recv_data, bytes_read);
#ifdef DEBUG_UDP_SOCKET
  cout<<"in UdpSocket::ReceiveFrom: "<<" source:"<<ip<<" port:"<<port<<" data size:"<<data.size()<<"\t"<<bytes_read<<endl;
  size_t sz = data.size();
  if(sz <= 20) cout<<"data: "<<data<<endl;
  else cout<<"data first 10 bytes:"<<data.substr(0, 10)<<" data last 10 bytes"<<data.substr(sz - 10, 10)<<endl;
#endif
  return bytes_read;  
}
