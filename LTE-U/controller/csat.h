#ifndef CSAT_H_
#define CSAT_H_

#include "headers.h"
#include "wifi_packet.h"

class CSAT {
private:
  double mu_thr1_;
  double mu_thr2_;
  double delta_tup_;
  double delta_tdown_;


  double t_on_min_;
  double t_on_max_;

  double alpha_mu_;

  double lastmu_;  
  double tlaston_;

  double delta_mu_;
  
  double delta_mu_threshold_;
  double multi_factor_;
  double mu_alpha_;

  deque<double> onmss_;
  deque<double> mus_; 
public:
  CSAT();
  ~CSAT();
  void setThreshold(double muthr1, double muthr2);
  void setDelta(double up, double down);
  void setAlphaMU(double amu);

  void setDeltaParameters(double alpha);

  double calculateMin(int winodes, int interval);
  double calculateCSATMU(vector<WiFiPacket> *pkts, double totalofftime);
  double groundtruthMU(vector<WiFiPacket> *pkts, double totalofftime);
  int getOnCycle(int offms, int interval, double mu, int winodes);


};

#endif


/**
 * CSAT parameter selection:
 * Medium utilization: 
 */
