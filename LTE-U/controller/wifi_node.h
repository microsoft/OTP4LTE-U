#ifndef WIFINODE_H_
#define WIFINODE_H_

#include "headers.h"
#include "wifi_packet.h"


enum NodeType {
  UNKNOWN = -1,
  AP = 0,
  STA = 1
};


class WiFiNode {
private:
  string mac_;
  int pktsent_;
  int rssi_;
  NodeType type_;
  //useful only if the node is a client and
  //it is associated with an AP
  string apmac_; 

  //for virtual AP
  bool virtualAP_;
public:
  WiFiNode();
  WiFiNode(string mac);
  ~WiFiNode();

  void setMAC(string mac) {mac_ = mac;}
  void setAP(string ap) {apmac_ = ap;}
  void setType(NodeType type) {type_ = type;}

  void setVirtual() {virtualAP_ = true;}
  
  NodeType type() const {return this->type_;}
  string mac() const {return this->mac_;}
  string ap() const {return this->apmac_;}
  bool isVirtualAP() const {return this->virtualAP_;}

  int rssi() const {return this->rssi_;}
  int pktsent() const {return this->pktsent_;}

  void update(WiFiPacket &pkt);
};

#endif
