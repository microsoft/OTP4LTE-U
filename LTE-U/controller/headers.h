#ifndef HEADERS_H_
#define HEADERS_H_


//#define _GNU_SOURCE

/*General C Headers*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>     
#include <assert.h>

#include <netinet/in.h>
#include <net/bpf.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>


#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 



/*General C++ Headers*/


#include <iostream>
#include <sstream>
#include <fstream>

#include <vector>
#include <deque>
#include <map>
#include <list>

#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>

using namespace std;


template < typename T > std::string to_string(const T& n ) {
  std::ostringstream stm ;
  stm << n ;
  return stm.str() ;
}

const int kCSATPacketDuration = 4; //in ms

#endif

