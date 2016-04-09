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

#include "monitor.h"

Monitor::Monitor()
{
  this->packets_ = new vector<WiFiPacket>();
  this->rt_ = false;
}


Monitor::~Monitor()
{
  delete this->packets_;
}

string Monitor::getJsonMessage(int interval, int ontime)
{

  Json::Value data;
  data["oncycle"] = ontime;
  data["offcycle"] = interval - ontime;
  data["interval"] = interval;
  string str = data.toStyledString();
  return str;
}



void Monitor::classifyNodeType(map<string, NodeType> &nodes, WiFiPacket &pkt)
{
  map<string, NodeType>::iterator it;
  map<string, double>::iterator muit;
  string tmac = pkt.tmac();
  string rmac = pkt.rmac();
  //if the address is already known, then simply return
  it = nodes.find(tmac);
  if(it == nodes.end()) {
    nodes[tmac] = UNKNOWN;
  }
  it = nodes.find(rmac);
  if(it == nodes.end()) {
    nodes[rmac] = UNKNOWN;
  }
  //classify the node based on the packet type
  switch(pkt.type()) {
    case BEACON:
      nodes[tmac] = AP;
    break;
    case PROBE_REQUEST:
      nodes[tmac] = STA;
    break;
    case PROBE_RESPONSE:
      nodes[tmac] = AP;
      nodes[rmac] = STA;
    break;
    
    case BLOCKACK:
    case BLOCKREQ:
    case QOSDATA:
    case RTS:
      if(nodes[tmac] == AP && nodes[rmac] == UNKNOWN) {
        nodes[rmac] = STA;
      }
      if(nodes[rmac] == AP && nodes[tmac] == UNKNOWN) {
        nodes[tmac] = STA;
      }
    break;
    default:
    break;
  } 
}


void Monitor::insertPacket(WiFiPacket &pkt)
{
  packets_->push_back(pkt);
  //classify wifi node type  
  classifyNodeType(this->types_, pkt);
  //
  string tmac = pkt.tmac();
  string rmac = pkt.rmac();
  nodes_[tmac].setMAC(tmac);
  nodes_[tmac].setType(types_[tmac]);
  nodes_[tmac].update(pkt);

  nodes_[rmac].setMAC(rmac);
  nodes_[rmac].setType(types_[rmac]);
  //ap may be updated multiple times, e.g., using probe requests to update
  if(nodes_[tmac].type() == STA && nodes_[rmac].type() == AP) {
    nodes_[tmac].setAP(rmac);
  } else if(nodes_[tmac].type() == AP && nodes_[rmac].type() == STA) {
    nodes_[rmac].setAP(tmac);
  } else {
  }

}

NodeType Monitor::getNodeType(string mac)
{
  map<string, NodeType>::iterator iter = types_.find(mac);
  if(iter != types_.end()) {
    return iter->second;
  } else {
    return UNKNOWN;
  }
}


void Monitor::identifyVirtualAPs(map<string, WiFiNode> &nodes) {
  map<string, WiFiNode>::iterator iter;
  if(nodes.size()==0) return;
  WiFiNode &lastNode = nodes_.begin()->second;
  int lastrssi = -1000;
  for(iter = nodes_.begin(); iter != nodes_.end(); ++iter) {
     WiFiNode& node = iter->second;
     string mac = iter->first;
     NodeType t = node.type();
     if(t!=AP) continue;
     //if two AP share the same MAC address prefix, then at least one should be virtuals
     //for virtual APs, we record its physical MAC address, as its AP mac
     if(mac.substr(0,10)==lastNode.mac().substr(0,10) && abs(lastNode.rssi() - node.rssi()) <= 3) {        
        node.setVirtual();
        if(lastNode.isVirtualAP()) {
          node.setAP(lastNode.ap());
        } else {
          node.setAP(lastNode.mac());
        } 
     }
     lastNode = node;
  }  
}

void Monitor::analyzeWiFiLinks()
{
  map<string, WiFiNode>::iterator iter;
  //identify the virual aps
  identifyVirtualAPs(nodes_);

  for(iter = nodes_.begin(); iter != nodes_.end(); ++iter) {
    WiFiNode node = iter->second;
    string mac = iter->first;
    NodeType t = node.type();
    if(t==AP && !node.isVirtualAP()) {
       //cout<<mac<<","<<node.rssi()<<","<<node.pktsent()<<","<<node.isVirtualAP()<<endl;       
       cout<<node.rssi()<<endl;       
    }
    //jump over
    continue;
    if(t==STA) {
      string apmac = node.ap();
      map<string, WiFiNode>::iterator tmp = nodes_.find(apmac);
      if(tmp != nodes_.end()) {
        WiFiNode ap = tmp->second;
        cout<<mac<<","<<node.rssi()<<","<<node.pktsent()<<","<<apmac<<","<<ap.rssi()<<","<<ap.pktsent()<<endl;
      } else {
        cout<<mac<<","<<node.rssi()<<","<<node.pktsent()<<","<<apmac<<endl;
      }
          
    }
  }
  
}


void Monitor::countWiFiNode()
{
  map<string, NodeType> nodes;
  int sz = packets_ -> size();
  int aps = 0, stas = 0, unknown = 0;
  NodeType t,r;
  for(int i = 0; i < sz; ++i) {
    WiFiPacket pkt = packets_->at(i);
    classifyNodeType(nodes, pkt);
  }
  map<string, NodeType>::iterator iter;
  for(iter = nodes.begin(); iter != nodes.end(); ++iter) {
    NodeType t = iter->second;
    string mac = iter->first;
    if(t==AP) aps++;
    else if(t==STA) stas++;
    else unknown++;
    //cout<<mac<<","<<t<<endl;
  }
  this->aps_ = aps;
  this->stas_ = stas; 
  //cout<<"APs:"<<aps<<", clients:"<<stas<<", unknown:"<<unknown<<endl;
}

void Monitor::calculateAPMU(vector<WiFiPacket> *pkts, double total)
{
 
  int sz = pkts->size();
  double sum = 0.0;
  int ctscc = 0, ackcc = 0, blkackcc = 0;
  for(int i = 0; i < sz; ++i) {
    WiFiPacket pkt = pkts->at(i);
    string tmac = pkt.tmac();
    string rmac = pkt.rmac();
    uint64_t now = pkt.time();
    double rate = pkt.rate();
    double len = pkt.packetLen();
    double tus = len*8/rate + pkt.preambleDuration();
    double curtus = tus;
    bool hasdata = false;
    double duration = 5000.0;
    double estimate = 0.0;
    bool dup = false;
    switch (pkt.type()) {
      case RTS:  
      break;
      case CTS:
       for(int j = i + 1; j < sz ;++j) {
          WiFiPacket next = pkts->at(j);
          if(0==estimate) {
            estimate = next.time() - now;
          }
          if(next.time() - now > duration) {
            break;
          }
          if(next.type() == QOSDATA && next.tmac() == pkt.rmac()) {
            hasdata = true;
            break;
          }
        } 
        if(!hasdata) {
          curtus += min((double)pkt.duration(), estimate);
        }
      break;
      case QOSDATA:
         for(int j = i - 1; j >= 0;--j) {
           WiFiPacket pre = pkts->at(j);
           if(now - pre.time() > duration) {
             break;
           }
           if(pre.type() == QOSDATA && pre.tmac() == pkt.tmac() && pre.rmac() == pkt.rmac()) {
             dup = true;
             break;
           }
         } 
         if(true==dup) {
           curtus -= pkt.preambleDuration();
         }
      break; 
      case ACK:
        for(int j = i - 1; j >= 0;--j) {
          WiFiPacket pre = pkts->at(j);
          if(0==estimate) {
            estimate = now - pre.time();
          }
          if(now - pre.time() > duration) {
            break;
          }
          if(pre.type() == QOSDATA && pre.tmac() == pkt.rmac()) {
            hasdata = true;
            break;
          }
          if(pre.type() == CTS && pre.rmac() == pkt.rmac()) {
            //does not have data, but use CTS already, avoid double calculation
            hasdata = true;
            break;
          }
        } 
        if(!hasdata) {
          //sum += kCSATPacketDuration;
         //cout<<"ACK: duration,"<<", estimate"<<estimate<<endl;
          curtus += min(4000.0, estimate); 
        }        
      break;
      case BLOCKREQ:
         
      break;
      case BLOCKACK:
        for(int j = i - 1; j >= 0;--j) {
          WiFiPacket pre = pkts->at(j);
          if(0==estimate) {
            estimate = now - pre.time();
          }
          if(now - pre.time() > duration) {
            break;
          }
          if(pre.type() == QOSDATA && pre.tmac() == pkt.rmac()) {
            hasdata = true;
            break;
          }
          if(pre.type() == CTS && pre.rmac() == pkt.rmac()) {
            hasdata = true;
            break;
          }
        } 
        if(!hasdata) {
          curtus += min(4000.0,estimate); 
        }        
      break;
      default:
      break;
    }
    ////////////calculate the MU of each AP////////////////////
    sum += curtus;
    string apmac = getAPMAC(pkt, types_, nodes_);
    if(apmu_.find(apmac) != apmu_.end()) {
      apmu_[apmac] += curtus;
    } else {
      apmu_[apmac] = curtus; 
    }
    /////////////////////////////////
  }
  static const string tplink = "60e327813766";
  static const string openwrt = "28c68eb3efcf";
  for (map<string, double>::iterator muit = apmu_.begin(); muit != apmu_.end(); muit++) {
    if(muit->first == tplink || muit->first == openwrt) { 
       cout<<muit->first<<","<<muit->second/total<<";";
    }
    muit->second = 0;
  }
  cout<<sum/total<<endl;
}

string Monitor::getAPMAC(WiFiPacket &pkt, map<string, NodeType> &types, map<string, WiFiNode> &nodes)
{
  string tmac = pkt.tmac();
  string rmac = pkt.rmac();
  if (types[tmac] == AP) return tmac;
  if (types[rmac] == AP) return rmac;
  if (types[tmac] == STA) {
    string tmp = nodes[tmac].ap();
    if (types[tmp] == AP) return tmp;
  }
  if (types[rmac] == STA) {
    string tmp = nodes[rmac].ap();
    if (types[tmp] == AP) return tmp;
  }
  return "";   
}




