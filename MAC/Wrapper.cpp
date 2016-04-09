#define INLINED

#include <sora_ip.c>
#include <sora_ext_lib.cpp>
#include <sora_ext_viterbi.cpp>
#ifndef WIN64
	#include "../Library/C/ChanDecode.c"
	#include <sora_ext_visual.c>
#endif
#include <bit.c>
#include <wpl_alloc.c>
#include <numerics.c>
#include <buf_bit.c>
#include <buf_chunk.c>
#include <buf_numerics8.c>
#include <buf_numerics16.c>
#include <buf_numerics32.c>
#include <sora_threads.c>
#include <sora_thread_queues.c>
#include <ext_arr.c>
#include <ext_math.c>
#include <utils.c>
#include "params.c"
#include "RegisterRW.cpp"
#include "threads.c"
#include "sora_radio.c"
#ifdef BLADE_RF
  #include <bladerf_radio.c>
#endif
#include "mac_utils.c"
#include "mac_tx_out_queue.c"
#include "mac_msg_parsing.c"
#include "mac_2threads.c"
#include "mac_rx.c"
#include "mac_tx_helper.c"
#include "mac_tx.c"
#include "txrx_sync.c"
#include "driver.c"
#include "tx.c"
#include "rx.c"
#ifdef LTE_U
#include "mac_udpsocket.c"
#include "jsmn.c"
#endif
