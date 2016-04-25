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

#include "csat.h"

CSAT::CSAT()
{
  t_on_max_ = 20; //ms
  t_on_min_ = 4;  //ms
  lastmu_ = -1;
  tlaston_ = -1;


  setThreshold(0.44, 0.67);
  setDelta(1.0, 1.0);
  setAlphaMU(0.7);

 
  //delta MU based
  multi_factor_ = 0.25; 
   // on time increase factor 12->15, 16->18 etc. 
  setDeltaParameters(0.1);
}

CSAT::~CSAT()
{
  
}



void CSAT::setThreshold(double muthr1, double muthr2) 
{
  mu_thr1_ = muthr1; 
  mu_thr2_ = muthr2;
}
  
void CSAT::setDelta(double up, double down) 
{
  delta_tup_ = up; 
  delta_tdown_ = down;
}
  
void CSAT::setAlphaMU(double amu) 
{
  alpha_mu_ = amu;
}

void CSAT::setDeltaParameters(double alpha)
{
  mu_alpha_ = alpha;
}


double CSAT::calculateMin(int winodes, int interval)
{
  const int n = 0;
  const int m = 0;
  //cout<<(double)((n + 1) * interval)/(double)(n + 1 + m + winodes)<<endl;
  double tonmin = max(t_on_min_, (double)((n + 1) * interval)/(double)(n + 1 + m + winodes));
  return tonmin; 
}


// Original Qualcomm CSAT
int CSAT::getOnCycle(int offms, int interval, double mu, int winodes)
{
  double ton = 0.0;
  double newmu = 0.0;
  if(lastmu_ < 0) {
    newmu = mu;
  } else {
    newmu = alpha_mu_ * mu + (1 - alpha_mu_) * lastmu_;
  }
  if(tlaston_ < 0) {
    tlaston_ = interval - offms;
  }
  double calmin = calculateMin(winodes, interval);
  //cout<<"mu:"<<mu<<",mu avg:"<<newmu<<endl;
  if(newmu < mu_thr1_) {
    ton = min(tlaston_ + delta_tup_, t_on_max_);
  } else if (newmu <= mu_thr2_) {
    ton = tlaston_;
  } else {
    ton = max(tlaston_ - delta_tdown_, calmin);
  }  
  lastmu_ = newmu;
  tlaston_ = ton;
  return round(ton);
}



double CSAT::calculateCSATMU(vector<WiFiPacket> *pkts, double total)
{
  int sz = pkts->size();
  double sum = 0.0;
  int ctscc = 0, ackcc = 0, blkackcc = 0;
  for(int i = 0; i < sz; ++i) {
    WiFiPacket pkt = pkts->at(i);
    uint64_t now = pkt.time();
    double rate = pkt.rate();
    double len = pkt.packetLen();
    double tus = len*8/rate + pkt.preambleDuration();
    sum += tus; 
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
          sum += min((double)pkt.duration(), estimate);
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
           sum -= pkt.preambleDuration();
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
          sum += min(4000.0, estimate); 
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
          //sum += kCSATPacketDuration;
          //cout<<"BLOCK ACK: "<<", estimate"<<estimate<<endl;
          sum += min(4000.0,estimate); 
        }        
      break;
      default:
      break;
    }
  }
  return sum/total;
}


double CSAT::groundtruthMU(vector<WiFiPacket> *packets, double total)
{
  double sum = 0.0;
  int sz = packets->size(); 
  for(int i = 0; i < sz; ++i) {
    WiFiPacket pkt = packets->at(i);
    sum += pkt.packetLen() * 8/pkt.rate();
    sum += pkt.preambleDuration();
    uint64_t now = pkt.time();
    double duration = 1000;
    bool dup = false;    
    if (pkt.type() == QOSDATA) {
      for(int j = i - 1; j >= 0;--j) {
         WiFiPacket pre = packets->at(j);
         if(now - pre.time() > duration) {
           break;
         }
         if(pre.type() == QOSDATA && pre.tmac() == pkt.tmac() && pre.rmac() == pkt.rmac()) {
           dup = true;
           break;
         }
       } 
       if(true==dup) {
         sum -= pkt.preambleDuration();
       }  
    } 
  }
  return sum/total;
}



