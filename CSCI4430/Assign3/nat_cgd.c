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


extern "C" {
#include <linux/netfilter.h> // required by NF_ACCEPT, NF_DROP, etc...
#include <libnetfilter_queue/libnetfilter_queue.h>
}

#include "checksum.h"
#include <math.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define BUF_SIZE 1500

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct nat_entry // ip and ports are 0 if available
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
int num_token = 0;

int bucket_size = 0;
int fill_rate = 0;
int fill_per_msec;
unsigned long long last_time;

int port_av[2001];

struct verdict_para
{ 
  struct nfq_q_handle * queue;
  unsigned int id;
  int ip_pkt_len;
};

unsigned char bufs[10][BUF_SIZE];
int buf_av[10]; // buf avaiable 
struct verdict_para verdict_table[10];




int fill_token(int num_token){
  struct timeval time;     
  gettimeofday(&time, NULL);
  // cur_time in unit ms
  unsigned long long cur_time = time.tv_sec * 1000 + time.tv_usec / 1000; 

  // last_time is the time that filled last token, not cur_time
  int num_fill = floor((double)(cur_time - last_time) / 1000.0 * fill_rate);
  if (num_fill > 0){
    last_time += round(1000 * (((double)num_fill) / fill_rate));
  }

  // fill a token if the bucket is not full, return current #token
  return min(bucket_size, num_token + num_fill);
}

// tshark -s 512 -i eth1 -f "ip.src == 137.189.88.148"


// void print_nat(){
//   int i;
//   for(i = 0; i < 2001; i++){
//     if(nat_table[i].internal_ip != 0){
//       printf("[NAT Entry] org addr:%d.%d.%d.%d, org port:%d | tran addr:%d.%d.%d.%d, tran port:%d\n",
//         (nat_table[i].internal_ip >> 24) & 0xFF, (nat_table[i].internal_ip >> 16) & 0xFF, (nat_table[i].internal_ip >> 8) & 0xFF, (nat_table[i].internal_ip) & 0xFF,
//         nat_table[i].internal_port,
//         (public_ip >> 24) & 0xFF, (public_ip >> 16) & 0xFF, (public_ip >> 8) & 0xFF, (public_ip) & 0xFF, 
//         nat_table[i].translated_port);
//     }
//   }
//   printf("\n");
// }

void print_nat(){
  int i;
  printf("[NAT Table Begin]\n");
  for(i = 0; i < 2001; i++){
    if(nat_table[i].internal_ip != 0){
      printf("[NAT Entry] org addr:%d.%d.%d.%d, org port:%d | tran addr:%d.%d.%d.%d, tran port:%d\n",
        (nat_table[i].internal_ip >> 24) & 0xFF, (nat_table[i].internal_ip >> 16) & 0xFF, (nat_table[i].internal_ip >> 8) & 0xFF, (nat_table[i].internal_ip) & 0xFF,
        nat_table[i].internal_port,
        (public_ip >> 24) & 0xFF, (public_ip >> 16) & 0xFF, (public_ip >> 8) & 0xFF, (public_ip) & 0xFF, 
        nat_table[i].translated_port);
    }
  }
  printf("[NAT Table End]\n");
}


static int Callback(struct nfq_q_handle *myQueue, struct nfgenmsg *msg,
    struct nfq_data *pkt, void *cbData) {
  // call back is recv thread
  printf("callback start\n");
  int i;

  // Get the id in the queue
  unsigned int id = 0;
  struct nfqnl_msg_packet_hdr *header;
  if ((header = nfq_get_msg_packet_hdr(pkt))){
    id = ntohl(header->packet_id);
  } 
  
  // Access IP Packet
  unsigned char *pktData;
  int ip_pkt_len;

  ip_pkt_len = nfq_get_payload(pkt, &pktData); 
  printf("ip_pkt_len:%d\n", ip_pkt_len);
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

  // find one available buf
  int buf_av_idx = -1;
  for(i = 0; i < 10; i++){
    if(buf_av[i] == 0){
      buf_av_idx = i;
      break;
    }
  }

  if(buf_av_idx == -1){ // no buffer space
    printf("No buffer available\n");
    nfq_set_verdict(myQueue, id, NF_DROP, ip_pkt_len, pktData);
    return 1;
  }
  
  printf("try to get lock\n");
  pthread_mutex_lock(&mutex);
  printf("get lock\n");
  memcpy(bufs[buf_av_idx], pktData, ip_pkt_len);
  verdict_table[buf_av_idx].queue = myQueue;
  verdict_table[buf_av_idx].id = id;
  verdict_table[buf_av_idx].ip_pkt_len = ip_pkt_len;
  buf_av[buf_av_idx] = 1;

  printf("print buf_av\n");
  for(int idx=0 ;idx < 10;idx++){
    printf("%d ", buf_av[idx]);
  }
  pthread_mutex_unlock(&mutex);
  printf("release lock\n\n");

  printf("\ncallback end\n");

  return 1;
}

void *process_thread(void *arg){
  printf("processing thread\n");
  struct timespec tim1, tim2;
  tim1.tv_sec = 0;
  tim1.tv_nsec = fill_per_msec;
  int flag_success = 0;
  int idx;
  struct iphdr *iph;
  //iph->saddr;
  //iph->daddr;
  //iph->check;
  struct udphdr *udph;
  //udph->source;
  //udph->dest;
  //udph->check;
  unsigned int local_mask = 0xffffffff << (32-mask);
  int i;
  //mark all the port to be available
  // for(i = 0 ;i < 2001;i++){
  //   port_av[i] = 1; //???????????
  // }

  // record initial last_time as the time just consume the first token
  printf("set initial last_time\n");
  int last_time_initial_flag = 0;
  while (last_time_initial_flag == 0){
    pthread_mutex_lock(&mutex);
    for(idx=0 ;idx < 10;idx++){
      if (buf_av[idx] == 1){
        struct timeval time;     
        gettimeofday(&time, NULL);
        long long int last_time = time.tv_sec * 1000 + time.tv_usec / 1000;
        last_time_initial_flag = 1;
        break;
      }
    }
    pthread_mutex_unlock(&mutex);
  }

  printf("start filling and consuming tokens\n");
  while(1){
  while((num_token=fill_token(num_token))>=1){
    for(idx =0 ;idx < 10;idx++){
      if (buf_av[idx] == 1){
        // use one token for processing one packet
        num_token--;
        printf("consume 1 token, now %d tokens\n", num_token);
        int j;

        // remove expired entry
        struct timeval cur_time;
        gettimeofday(&cur_time, NULL);
        long long int cur_time_msec = cur_time.tv_sec * 1000 + cur_time.tv_usec / 1000;
        for (j = 0;j < 2001;j++){
          struct timeval nat_timestamp = nat_table[j].timestamp;
          if(nat_table[j].internal_ip == 0) continue;
          if (cur_time_msec - (nat_timestamp.tv_sec*1000 + nat_timestamp.tv_usec/1000) > 10000){
            unsigned long long tmp_time = timestamp.tv_sec * 1000 + timestamp.tv_usec / 1000;
            printf("expired time: %llu\n", tmp_time);
            int tmp_translated_port = nat_table[j].translated_port;
	          nat_table[j].translated_port =0 ;
            nat_table[j].internal_ip = 0;
            nat_table[j].internal_port = 0;
            // nat_table[i].timestamp = NULL;
            port_av[tmp_translated_port-10000] = 0;
            print_nat();
          }
        }

        unsigned char *pktData = bufs[idx];
        struct iphdr *ipHeader = (struct iphdr *)pktData;
      
        iph = (struct iphdr*)pktData;
        udph = (struct udphdr*) (((char*)ipHeader) + ipHeader->ihl*4);
        // judge whether it is a inbound or outbound packet

        if ((ntohl(iph->saddr) & local_mask) == lan) {
          printf("outbound traffic\n");
          // outbound traffic
          // lookup the nat table
          int flag_in_nat_table = 0; 
          for (j = 0 ;j < 2001;j++){
            if ( (nat_table[j].internal_ip == ntohl(iph->saddr)) && (nat_table[j].internal_port == ntohs(udph->source)) ){
              flag_in_nat_table = 1;
              int translated_port = nat_table[j].translated_port;
              iph->saddr = htonl(public_ip);
              udph->source = htons(translated_port);

              // printf("checksum before htons, udp=0x%x, ip=0x%x\n", udp_checksum(pktData), ip_checksum(pktData));
              udph->check = udp_checksum(pktData);
              iph->check = ip_checksum(pktData); 
              // printf("checksum before htons, udp=0x%x, ip=0x%x\n", htons(udp_checksum(pktData)), htons(ip_checksum(pktData)));

              struct timeval timestamp;
              gettimeofday(&timestamp, NULL);
              nat_table[j].timestamp = timestamp;
              unsigned long long time = timestamp.tv_sec * 1000 + timestamp.tv_usec / 1000;
              printf("used timestamp: %llu\n", time);
              break;
            }
          }
	  
          if (!flag_in_nat_table){
            // find an available port 
            printf("not in table\n");
	          int translated_port = 0;
            for (j = 0;j < 2001;j++){
              if(port_av[j] == 0){
                translated_port = j+10000;
                break;
              }
            }
            // create new entry
            for(j = 0;j < 2001;j++){
              if(nat_table[j].translated_port == 0){
                // store new entry in table
                nat_table[j].translated_port = translated_port;
                nat_table[j].internal_port = ntohs(udph->source);
                nat_table[j].internal_ip = ntohl(iph->saddr);
                
                // change packet ip
                iph->saddr = htonl(public_ip);
                udph->source = htons(translated_port);
                // printf("checksum before htons, udp=0x%x, ip=0x%x\n", udp_checksum(pktData), ip_checksum(pktData));

                udph->check = udp_checksum(pktData);
                iph->check = ip_checksum(pktData);
                
                // printf("checksum after htons, udp=0x%x, ip=0x%x\n", htons(udp_checksum(pktData)), htons(ip_checksum(pktData)));

                // set port unavailable
                port_av[translated_port-10000] = 1;

                // initialize new entry's timestamp
                struct timeval timestamp;
                gettimeofday(&timestamp, NULL);
                nat_table[j].timestamp = timestamp;
                unsigned long long time = timestamp.tv_sec * 1000 + timestamp.tv_usec / 1000;
                printf("used timestamp: %llu\n", time);
                print_nat();
                break;
              }
            }
          }

        }else{
          printf("inbound traffic\n");
          // inbound traffic
          int flag_in_nat_table = 0;
          // int flag_valid = 0;
          for (j = 0 ;j < 2001;j++){
            if ( (nat_table[j].translated_port == ntohs(udph->dest)) && (public_ip == ntohl(iph->daddr)) ){
              flag_in_nat_table = 1;
              int internal_port = nat_table[j].internal_port;
              int internal_ip = nat_table[j].internal_ip;
              iph->daddr = htonl(internal_ip);
              udph->dest = htons(internal_port);

              udph->check = udp_checksum(pktData);
              iph->check = ip_checksum(pktData);

              struct timeval timestamp;
              gettimeofday(&timestamp, NULL);
              nat_table[j].timestamp = timestamp;
              break;
            }
          }
          printf("end checking table\n");

          if (!flag_in_nat_table){
            printf("cannot find an entry\n");
            memset(bufs[idx], 0, BUF_SIZE);
            pthread_mutex_lock(&mutex);
            // no entry, directly drop from user space buf_av
            buf_av[idx] = 0;
            pthread_mutex_unlock(&mutex);
            break;
          }
        }
        
        printf("find an entry\n");
        // send the packet
        if((flag_success = nfq_set_verdict(verdict_table[idx].queue, verdict_table[idx].id, NF_ACCEPT, 
          verdict_table[idx].ip_pkt_len, pktData))<0){
          fprintf(stderr,"nfq_set_verdict error\n");
        }
        pthread_mutex_lock(&mutex);
        buf_av[idx] = 0;
        pthread_mutex_unlock(&mutex);
        break;
      }
    }
  }
  }
  printf("exit processing thread\n");
}



int main(int argc, char** argv) {

  if(argc != 6){
    fprintf(stderr, "Arg num error\n");
    exit(-1);
  }

  struct in_addr public_ip_addr;
  inet_aton(argv[1], &public_ip_addr);
  public_ip = ntohl(public_ip_addr.s_addr);

  struct in_addr lan_ip_addr;
  inet_aton(argv[2], &lan_ip_addr);
  lan = ntohl(lan_ip_addr.s_addr);

  mask = atoi(argv[3]);
  bucket_size = atoi(argv[4]);
  num_token = bucket_size;
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

  // create 1 threads:
  // 1 for processing thread 
  pthread_t pro_thread; // thread for processing 
  pthread_create(&pro_thread, NULL, process_thread, NULL);
  pthread_detach(pro_thread);

  printf("recv\n");
  // recv thread as main 
  while ((res = recv(fd, buf, sizeof(buf), 0)) && res >= 0) {
    nfq_handle_packet(nfqHandle, buf, res);
  }

  nfq_destroy_queue(nfQueue);
  nfq_close(nfqHandle);

}
