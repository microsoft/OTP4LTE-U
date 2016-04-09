#ifndef UDP_SOCKET_
#define UDP_SOCKET_

#include "headers.h"

//#define DEBUG_UDP_SOCKET 1

const int32_t kPacketSize = 1000;

class UdpSocket {

private:
  int32_t udp_socket_id_;

  /** 
   * The address and port number this socket bind to
   * it should bind to a broadcast address to receive broadcast message
   */
  struct sockaddr_in udp_addr_;
  int32_t udp_port_;

  int32_t read_packet_size_;
protected:
  /** Broadcast Address and port */
  struct sockaddr_in udp_broadcast_addr_;
  int32_t udp_broadcast_port_;
  bool broadcast_server;

  /***/
  struct sockaddr_in udp_client_addr_;
 
public:
  UdpSocket(int32_t pkt_sz = kPacketSize);
  ~UdpSocket();

  int32_t SetNonblocking();

//====================================================================================================
  /**
   * This UDP socket bind to a ip address (usually itself or a broadcast address) and port number
   * The ip should be a broadcast ip to receive broadcast message
   * @param [in] ip: the ip address this socket bind to
   * @param [in] port: the port number this socket bind to
   */
  void UdpSocketSetUp(const string& ip, const int32_t& port);

//====================================================================================================
  /**
   * Once the socket bind to a ip address, it is able to send packet to any other UDP socket
   * with ip address and port number
   * @param [in] ip: the ip address of destination
   * @param [in] port: the port number of destination
   * @param [in] data: the data send to the destination
   */ 
  int32_t SendTo(const string& ip, const int32_t& port, const string& data);

//====================================================================================================
  /**
   * Once the socket bind to a ip address and a port number, it is able to hear that port
   * @param [out] ip: the ip address of the packet source
   * @param [out] port: the port number of the packet source
   * @param [out] data: the data from the packet source
   * @return the number of bytes received from the source
   */ 
  int32_t ReceiveFrom(string& ip, int32_t& port, string& data);

//====================================================================================================

  /**
   * This UDP socket set up a broadcast ip address and port number
   * @param [in] ip: the broadcast ip address
   * @param [in] port: the broadcast port number
   */
  void UdpSocketSetUpBroadcast(const string& ip, const int32_t& port);

//====================================================================================================
  /**
   * This can be achieved by SendTo(broadcastIp, broadcastPort, data) after invoke UdpSocketSetUpBroadcast()
   * @param [in] data: the broadcast message
   */
  int32_t UdpSocketBroadcast(const string& data);


};

#endif
