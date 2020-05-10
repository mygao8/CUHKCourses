#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h> // required by "netfilter.h"
#include <arpa/inet.h> // required by ntoh[s|l]()
#include <signal.h> // required by SIGINT
#include <string.h> // required by strerror()
#include <sys/time.h> // required by gettimeofday()
#include <time.h> // required by nanosleep()
#include <errno.h> // required by errno
# include <pthread.h>
#include <netinet/ip.h>        // required by "struct iph"
#include <netinet/tcp.h>    // required by "struct tcph"
#include <netinet/udp.h>    // required by "struct udph"
#include <netinet/ip_icmp.h>    // required by "struct icmphdr"
#include <math.h>


extern "C" {
#include <linux/netfilter.h> // required by NF_ACCEPT, NF_DROP, etc...
#include <libnetfilter_queue/libnetfilter_queue.h>
}

#include "checksum.h"

#define BUF_SIZE 1500

#define min(a, b) ((a) < (b) ? (a) : (b))


struct  nat_entry // ip and ports are 0 if available
{
  int internal_ip; 
  int internal_port;
  int translated_port;
  struct timeval timestamp;
};


struct nat_entry nat_table[2001]; // nat entries of size 2001 (10000-12000)

// input args 
int public_ip; // inet_aton(<IP>)
int lan;
int mask;
int token_bucket = 0;

int bucket_size = 0;
int fill_rate = 0;
int fill_per_msec;

int port_av[2001];

struct verdict_para
{ 
  (struct nfq_q_handle *) queue,
  unsigned int id,
  int ip_pkt_len, 
};

unsigned char bufs[10][BUF_SIZE];
int buf_av[10]; // buf avaiable 
struct verdict_para verdict_table[10];


int fill_token(long long int *last_time, int bucket_size, int fill_rate, int num_token){
  struct timeval time;     
  gettimeofday(&time, NULL);
  long long int cur_time = time.tv_sec * 1000 + time.tv_usec / 1000;

  // last_time is the time that filled last token, not cur_time
  double num_fill = floor((double)(cur_time - last_time) / 1000.0 * fill_rate);
  if (num_fill >= 10e-9){
    *last_time += (int) round(1000 * (num_fill / fill_rate));
  }
  
  // fill a token if the bucket is not full, return current #token
  return min(bucket_size, num_token + num_fill);
}

int print_nat(){
  int i;
  for(i = 0; i < 2001; i++){
    if(nat_table[i].internal_ip != 0){
      printf("[NAT Entry] org addr:%d.%d.%d.%d, org port:%d | tran addr:%d.%d.%d.%d, tran port:%d\n",
        (nat_table[i].internal_ip >> 24) & 0xFF, (nat_table[i].internal_ip >> 16) & 0xFF, (nat_table[i].internal_ip >> 8) & 0xFF, (nat_table[i].internal_ip) & 0xFF,
        nat_table[i].internal_port,
        (public_ip >> 24) & 0xFF, (public_ip >> 16) & 0xFF, (public_ip >> 8) & 0xFF, (public_ip) & 0xFF, 
        nat_table[i].translated_port);
    }
  }
  printf("\n");
}


static int Callback(struct nfq_q_handle *myQueue, struct nfgenmsg *msg,
    struct nfq_data *pkt, void *cbData) {
  // call back is recv thread
  
  int i;
  struct timeval cur_time;
  time(&cur_time);
  // remove expired entry
  for(i = 0; i < 2001; i++){
    if(nat_table[i].internal_ip != 0){
      if((cur_time.tv_sec - nat_table[i].timestamp.tv_sec >= 10) && (cur_time.tv_usec - nat_table[i].timestamp.tv_usec >= 0) ){
        port_av[nat_table[i].translated_port - 10000] = 0;
        nat_table[i].internal_ip = 0;
        nat_table[i].internal_port = 0;
        nat_table[i].translated_port = 0;
        print_nat();
      }
    }
  }

  // find one available entry
  int nat_av_idx = -1;
  for(i = 0; i < 2001; i++){
    if(nat_table[i].internal_ip == 0){
      nat_av_idx = i;
      break;
    }
  }

  // find one available port
  int port_av_idx = -1;
  for(i = 0; i < 2001; i++){
    if(port_av[i] == 0){
      port_av_idx = i;
      break;
    }
  }

  // find one available buf
  int buf_av_idx = -1;
  for(i = 0; i < 10; i++){
    if(buf_av[i] == 0){
      buf_av_idx = i;
      break;
    }
  }

  // Get the id in the queue
  unsigned int id = 0;
  struct nfqnl_msg_packet_hdr *header;
  if (header = nfq_get_msg_packet_hdr(pkt)) 
    id = ntohl(header->packet_id);

  // Access IP Packet
  unsigned char *pktData;
  int ip_pkt_len;
  struct iphdr *ipHeader;

  ip_pkt_len = nfq_get_payload(pkt, &pktData); 
  if(ip_pkt_len > 1500){
    printf("Packet larger than 1500!\n");
  }
  struct iphdr *ipHeader = (struct iphdr *)pktData;

  // Access UDP Packet
  struct udphdr *udph = (struct udphdr *) (pktData + ipHeader->ihl * 4) ;

  // Access App Data
  unsigned char* appData;
  int udp_payload_len; // prompt: udp_payload_len + udp_header_len + ip_header_len = ip_pkt_len



  if(ipHeader->protocol != IPPROTO_UDP) { // drop packets not in UDP
    printf("Packet drop: not UDP\n");
    nfq_set_verdict(myQueue, id, NF_DROP, ip_pkt_len, pktData);
    return 1;
  }

  if(nat_av_idx == -1){ // no available entry in nat table
    fprintf(stderr, "ERROR: Drop packet: nat table full!\n");
    nfq_set_verdict(myQueue, id, NF_DROP, ip_pkt_len, pktData);
    return 1;
  }

  if(buf_av_idx == -1){ // no buffer space
    printf("No buffer\n");
    nfq_set_verdict(myQueue, id, NF_DROP, ip_pkt_len, pktData);
    return 1;
  }

  if(port_av_idx == -1){
    printf("Port full\n");
    nfq_set_verdict(myQueue, id, NF_DROP, ip_pkt_len, pktData);
    return 1;
  }

  unsigned int local_mask = 0xffffffff << (32 – mask);
  if((ntohl(ipHeader->saddr) & local_mask) == lan ){
    // outbound
    int nat_match = -1;
    for(i = 0; i < 2001; i++){
      if(nat_table[i].internal_ip == ipHeader->saddr && nat_table[i].internal_port == udph->source){
        nat_match = i;
        break;
      }
    }

    if(nat_match == -1){
      nat_table[nat_av_idx].internal_ip = ipHeader->saddr;
      nat_table[nat_av_idx].internal_port = udph->source;
      nat_table[nat_av_idx].translated_port =  10000 + port_av_idx;
      port_av[port_av_idx] = 1;
      time(&cur_time);
      nat_table[nat_av_idx].timestamp = cur_time;
      print_nat();    
    }

    memcpy(bufs[buf_av_idx], pktData, ip_pkt_len);
    verdict_table[buf_av_idx].queue = myQueue;
    verdict_table[buf_av_idx].id = id;
    verdict_table[buf_av_idx].ip_pkt_len = ip_pkt_len;
    buf_av[buf_av_idx] = 1;

  }else{
    // inbound
    int nat_match = -1;
    for(i = 0; i < 2001; i++){
      if(public_ip == ipHeader->daddr && nat_table[i].translated_port == udph->dest){
        nat_match = i;
        break;
      }
    }
    if(nat_match == -1){
      // inbound cannot find nat entry in table
      // drop the packet
      printf("Inbound entry not found, drop packet\n");
      nfq_set_verdict(myQueue, id, NF_DROP, ip_pkt_len, pktData);
      return 1;
    }

    // inbound find entry in table
    memcpy(bufs[buf_av_idx], pktData, ip_pkt_len);
    verdict_table[buf_av_idx].queue = myQueue;
    verdict_table[buf_av_idx].id = id;
    verdict_table[buf_av_idx].ip_pkt_len = ip_pkt_len;
    buf_av[buf_av_idx] = 1;

  }

  return 1;
}

void process_thread(){

  // ...

  nfq_set_verdict(myQueue, id, NF_ACCEPT, ip_pkt_len, pktData);
}


int main(int argc, char** argv) {

  if(argc != 6){
    fprintf(stderr, "Arg num error\n");
  }

  public_ip = inet_aton(argv[1]);
  lan = inet_aton(argv[2]);
  mask = atoi(argv[3]);
  bucket_size = atoi(argv[4]);
  fill_rate = atoi(argv[5]);
  int fill_per_msec = 1000000/fill_rate;

  // initialize the nat table
  int i;
  for (i = 0; i < 2001; i++)
  {
    nat_table[i].internal_ip = 0;
    nat_table[i].internal_port = 0;
    nat_table[i].translated_port = 0;

    port_av[0] = 0;
  }
  // init buf_av[10]
  for(i = 0; i < 10; i++){
    buf_av[i] = 0;
  }

  // Get a queue connection handle from the module
  struct nfq_handle *nfqHandle;
  if (!(nfqHandle = nfq_open())) {
    fprintf(stderr, "Error in nfq_open()\n");
    exit(-1);
  }

  // Unbind the handler from processing any IP packets
  if (nfq_unbind_pf(nfqHandle, AF_INET) < 0) {
    fprintf(stderr, "Error in nfq_unbind_pf()\n");
    exit(1);
  }

  // Install a callback on queue 0
  struct nfq_q_handle *nfQueue;
  if (!(nfQueue = nfq_create_queue(nfqHandle,  0, &Callback, NULL))) {
    fprintf(stderr, "Error in nfq_create_queue()\n");
    exit(1);
  }
  // nfq_set_mode: I want the entire packet 
  if(nfq_set_mode(nfQueue, NFQNL_COPY_PACKET, BUF_SIZE) < 0) {
    fprintf(stderr, "Error in nfq_set_mode()\n");
    exit(1);
  }

  struct nfnl_handle *netlinkHandle;
  netlinkHandle = nfq_nfnlh(nfqHandle);

  int fd;
  fd = nfnl_fd(netlinkHandle);

  int res;
  char buf[BUF_SIZE];

  // create 2 threads:
  // 1 for token fill
  // 2 for processing thread 
  pthread_t pro_thread; // thread for processing 
  pthread_create(&pro_thread, NULL, process_thread, NULL);
  pthread_detach(pro_thread);

  // recv thread as main 
  while ((res = recv(fd, buf, sizeof(buf), 0)) && res >= 0) {
    nfq_handle_packet(nfqHandle, buf, res);
  }

  nfq_destroy_queue(nfQueue);
  nfq_close(nfqHandle);

}
