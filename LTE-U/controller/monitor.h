#ifndef MONITOR_H_
#define MONITOR_H_

#include <jsoncpp/json/json.h>
#include "headers.h"
#include "wifi_packet.h"
#include "wifi_node.h"
#include "udp_socket.h"
#include "csat.h"


class Monitor {
private:
  //meta information, always there
  //mac address of node, and the node type
  map<string, NodeType> types_;
  //mac address of client
  map<string, WiFiNode> nodes_;

  bool rt_;
  int aps_;
  int stas_;
  int links_;
  int sensing_;  
public:
  //buffer information, updated every cycle 
  pcap_t *handle_;
  pthread_mutex_t lock;  
  pthread_spinlock_t spinlock;

  void setSensingThreshold(int sensing) { sensing_ = sensing;}
  int getSensingThreshold() {return sensing_;}
  vector<WiFiPacket> *packets_;
  map<string, double> apmu_;
  string getAPMAC(WiFiPacket &pkt, map<string, NodeType> &types, map<string, WiFiNode> &nodes);

  void calculateAPMU(vector<WiFiPacket> *pkts, double total);
  void clearBuffer() {packets_->clear(); }

  void classifyNodeType(map<string, NodeType> &node, WiFiPacket &pkt);

  Monitor();
  ~Monitor();
  //the parsed wifi packets all go here
  void insertPacket(WiFiPacket &pkt);
  NodeType getNodeType(string mac);
  void enableRT() {this->rt_ = true;}
  bool isRT() {return this->rt_;}
  void countWiFiNode();

  void identifyVirtualAPs(map<string, WiFiNode> &node);
  void analyzeWiFiLinks();

  string getJsonMessage(int interval, int ontime);
  UdpSocket udpclient; 
  CSAT csat;
};


#endif
