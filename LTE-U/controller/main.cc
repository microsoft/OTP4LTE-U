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
#include "wifi_node.h"
#include "monitor.h"

const string kLocalIP = "192.168.1.10";
const int kLocalPort = 55555;
const string kRemoteIP = "192.168.1.11";
const int kRemotePort = 4444;

const bool kEnableSocket = true;



static void* WifiSniffer(void* monitor);
static void* LTEUScheduler(void* monitor);

void (*fptr)(u_char*, const struct pcap_pkthdr*, const u_char*);
void startCapture(bool live, string inputfile, void (*fptr)(u_char*, const struct pcap_pkthdr*, const u_char*), u_char *output);
void realtimeProcess(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet);


int kInitontime = 20;

int interval = 24; //in ms
int rounds = 20; //accmulate to monitor
int monitortime = interval*rounds;


int runtime = 5*60; //in second
static int counter = 0;

//input .pcap file name or monitor interface name
string wifile;
int ontime = kInitontime;

int totalon = 0;
int totaloff = 0;
 
int main(int argc, char* argv[])
{  
//  assert(argc >= 3);
  double thr1 = atof(argv[1]);
  double thr2 = atof(argv[2]);
  kInitontime = atoi(argv[3]);
  double alpha = atof(argv[4]);

  

  ontime = kInitontime;
  Monitor *monitor = new Monitor();
  //enable real time monitoring
  wifile = "mon0";
  monitor->enableRT();

  monitor->csat.setThreshold(thr1, thr2);
  monitor->csat.setDeltaParameters(alpha);
  monitor->setSensingThreshold(-62);
 
  pthread_spin_init(&monitor->spinlock, 0);
  
  int num = 2;
  pthread_t threads[num];
  pthread_create (&(threads[0]), NULL, &WifiSniffer, monitor);
  pthread_create (&(threads[1]), NULL, &LTEUScheduler, monitor);
  for(int i = 0; i < num; ++i) {
    pthread_join(threads[i], NULL);
  }
  pthread_spin_destroy(&monitor->spinlock);

  //monitor->analyzeWiFiLinks();

  delete monitor;

  cout<<"#,"<<totalon<<","<<totaloff<<endl; 
  return 0;
}

int payloadLength(vector<WiFiPacket> *packets)
{
  int sum = 0.0;
  int sz = packets->size(); 
  for(int i = 0; i < sz; ++i) {
    WiFiPacket pkt = packets->at(i);
   if (pkt.type() == QOSDATA) {
      sum += pkt.packetLen();
    } 
  }
  return sum;
}
 
static void* WifiSniffer(void* param)
{
  //fprintf(stdout, "Enter WiFiSniffer\n"); 
  Monitor *monitor = (Monitor*)param;
  startCapture(monitor->isRT(), wifile, &realtimeProcess, (u_char*)monitor);
  pthread_exit(NULL);
} 

static void* LTEUScheduler(void* param)
{
  //fprintf(stdout, "Enter LTEUScheduler\n");
  Monitor *monitor = (Monitor*)param;
  struct timeval begin;
  double scale = 0.0, now = 0.0, start;  
  double calculation = 2000.0;//us
  double latency = 2000.0; //us
  double mu = 0.0, gmu = 0.0;

  double sum[3] = {0.0, 0.0, 0.0};
  int cc = 0;
  while(true) {
       
    gettimeofday(&begin, NULL);  
    now = begin.tv_sec*1e6 + begin.tv_usec;  
    if(scale == 0) {
      scale = now;
      start = now;
    } 

    scale += monitortime*1000; 
    double sleepus = monitortime*1000 - calculation - latency;
    usleep(sleepus);
 
    int offtime = interval - ontime;
    //calculate 

    pthread_spin_lock(&monitor->spinlock);
    mu = monitor->csat.calculateCSATMU(monitor->packets_, rounds * 1000 * offtime);
    gmu = monitor->csat.groundtruthMU(monitor->packets_, rounds * 1000 * offtime);
    int payload = payloadLength(monitor->packets_);  //monitor->countWiFiNode();
//    monitor->calculateAPMU(monitor->packets_, rounds * 1000 * offtime);
  
    monitor->clearBuffer();
    pthread_spin_unlock(&monitor->spinlock);

    //statistics of last rounds
    totalon+=ontime*rounds;
    totaloff+=offtime*rounds;

    cc++;
    sum[0]+=gmu;
    sum[1]+=mu;
    sum[2]+=payload;
    gettimeofday(&begin, NULL);  
    now = begin.tv_sec*1e6 + begin.tv_usec;  

    cout<<(now-start)/1000<<","<<gmu<<","<<mu<<","<<ontime<<","<<offtime<<","<<payload<<endl;  
    
    if((now - start)/1e6 >= runtime) {
      cout<<"#,"<<sum[0]/(double)cc<<","<<sum[1]/(double)cc<<","<<totalon<<","<<totaloff<<","<<sum[2]<<endl;
      if(kEnableSocket) {
        monitor->udpclient.SendTo(kRemoteIP, kRemotePort, monitor->getJsonMessage(interval, kInitontime)); 
      }
      sleep(1);
      break;
    }   
    //send to LTEU    
    // Qualcomm CSAT
    ontime = monitor->csat.getOnCycle(offtime, interval, gmu, 1);    
/*
    static int counter = 0;
    ontime = kInitontime + (counter%2) * (offtime * 0.25);     
    counter++;
*/
   //prepare to send
    string msg = monitor->getJsonMessage(interval, ontime);
    gettimeofday(&begin, NULL);  
    now = begin.tv_sec*1e6 + begin.tv_usec;  
    if(now + latency < scale) {
      usleep(scale - now - latency);
    }
    if(true == kEnableSocket) {
      int res = monitor->udpclient.SendTo(kRemoteIP, kRemotePort, msg);
      if(res < 0) {
        cout<<"UDP Socket Error!!!"<<endl;
        break;
      }
    }
  }
  pcap_breakloop(monitor->handle_);  

  pthread_exit(NULL);
} 
void realtimeProcess(u_char *args, const struct pcap_pkthdr* pkthdr, const u_char* packet){
  Monitor* monitor = (Monitor*)args;
  WiFiPacket pkt;
  uint64_t now = pkthdr->ts.tv_sec * 1e6 + pkthdr->ts.tv_usec;
  pkt.parsePacket(now, pkthdr->len, packet);
/*  
  struct timeval begin;   
  gettimeofday(&begin, NULL);  
  uint64_t t = begin.tv_sec*1e6 + begin.tv_usec;  
  cout<<t-now<<endl;
*/
/*rate statistics
  static uint64_t beginning = 0;
  if(beginning==0) beginning = now;
  static string netgear = "28c68eb3efcf";
  static string tplink = "60e327813766";
  if(pkt.type() == QOSDATA && pkt.tmac() == tplink)
    cout<<now-beginning<<","<<pkt.rate()<<endl;
*/ 

  if(pkt.rssi() >= monitor->getSensingThreshold()) {
    pthread_spin_lock(&monitor->spinlock);
    counter++;
    monitor->insertPacket(pkt);
    pthread_spin_unlock(&monitor->spinlock);
  }
}



void startCapture(bool live, string input, void (*fptr)(u_char*, const struct pcap_pkthdr*, const u_char*), u_char *param) {
  const char *dev = input.c_str();			/* capture device name */
  char errbuf[PCAP_ERRBUF_SIZE];		/* error buffer */
  pcap_t *handle;  /* packet capture handle */
  pcap_stat ps;
#ifdef DUMP	  
  pcap_dumper_t *pcapfile; /* Pointer to a pcap file (opened or created) */
#endif
  
  char filter_exp[] = "";		/* filter expression [3] */
  struct bpf_program fp;			/* compiled filter program (expression) */
  bpf_u_int32 mask;			/* subnet mask */
  bpf_u_int32 net;			/* ip */
  int num_packets = 0;			/* number of packets to capture */
  int status = 0;
 /* open capture device */
  if(true==live) {
// pcap_open_live will buffer the packet
//    handle = pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);

    handle = pcap_create(input.c_str(), errbuf); 
    if (handle == NULL) {
      fprintf(stderr, "Couldn't open the device %s: %s\n", dev, errbuf);
      exit(EXIT_FAILURE);
    }
       
    status = pcap_set_buffer_size(handle, 1e8);
    if (0 != status) {
      fprintf(stderr, "Couldn't set buffer size: %s\n", errbuf);
      exit(EXIT_FAILURE);
    }   
    status = pcap_set_immediate_mode(handle, 1);
    if (0 != status) {
      fprintf(stderr, "Couldn't set immediate mode: %s\n", errbuf);
      exit(EXIT_FAILURE);
    }   
    status = pcap_activate(handle);
    if (0 != status) {
      fprintf(stderr, "Couldn't activate pcap: %s\n", errbuf);
      exit(EXIT_FAILURE);
    }   
 
  } else {
    handle = pcap_open_offline(input.c_str(), errbuf);  
    if (handle == NULL) {
      fprintf(stderr, "Couldn't find the file %s: %s\n", dev, errbuf);
      exit(EXIT_FAILURE);
    }
  }
#ifdef DUMP
  if ((pcapfile = pcap_dump_open(handle, output.c_str())) == NULL) {
    fprintf(stderr, "Error from pcap_dump_open(): %s\n", pcap_geterr(handle)); 
    exit(EXIT_FAILURE);
  }
#endif
  /* compile the filter expression */
  if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
    fprintf(stderr, "Couldn't parse filter %s: %s\n",filter_exp, pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }

  /* apply the compiled filter */
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
    exit(EXIT_FAILURE);
  }

  /* now we can set our callback function */
#ifdef DUMP
  if ((pcap_loop(handle, num_packets, pcap_dump, (u_char *)pcapfile)) != 0) {
    fprintf(stderr, "Error from pcap_loop(): %s\n", pcap_geterr(handle)); 
    //exit(EXIT_FAILURE);
  }
#else  
  Monitor *monitor = (Monitor*)param;
  monitor->handle_ = handle;
  if ((pcap_loop(handle, num_packets, fptr, param)) != 0) {
    fprintf(stdout, "#Exit from pcap_loop(): %s\n", pcap_geterr(handle)); 
    //exit(EXIT_FAILURE);
  }
#endif


  pcap_stats(handle, &ps);
  fprintf(stdout, "#PCAP stats: received packets: %d, dropped packets due to buffer overflow: %d, dropped packets by driver: %d\n", ps.ps_recv, ps.ps_drop, ps.ps_ifdrop);
  /* cleanup */
  pcap_freecode(&fp);
#ifdef DUMP
  pcap_dump_close(pcapfile);
#endif
  pcap_close(handle);

  runtime = 0;
//  fprintf(stdout, "exit pcap capture\n");
}


