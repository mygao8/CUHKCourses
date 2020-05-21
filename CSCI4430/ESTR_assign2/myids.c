#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>                                             
#include <pcap.h>                                             
#include <unistd.h>                                           
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <math.h>

#include "checksum.h"

#define abs(x) (((x)>=0) ? (x) : (-(x)))

#define ETH_HDR_LEN 14

enum flag_type {hitter, changer, spreader};

unsigned int pre_src_num = 0;
unsigned int src_num = 0; // num of source addrs

unsigned int * pre_src_addrs = NULL;	// previous epoch, addrs array for sources
unsigned int * pre_src_bytecount = NULL; // previous epoch, byte count array for sources
char * pre_detected_flag[3] = {NULL, NULL, NULL};

unsigned int * src_addrs = NULL;	// addrs array for sources
unsigned int * src_bytecount = NULL; // byte count array for sources
char * detected_flag[3] = {NULL, NULL, NULL};

unsigned int * src_dest_num = NULL; // dest num for a certain source
unsigned int ** src_dest_addrs = NULL; // dest addrs as array for a source, and for all sources as an array

int hh_thresh; // in MB
int hc_thresh; // in MB
int ss_thresh;

int first_epoch = 1;

/* The src_dest_addrs like:

		dest addrs
[0]  -> [     ][    ][    ]... size: src_dest_num[0]
[1]  -> [     ][    ][    ]... size: src_dest_num[1]
[2]  -> [     ][    ][    ]... size: src_dest_num[2]
...

*/

void print_ipaddr(int addr){
	printf("%d.%d.%d.%d", 
		(addr >> 24) & 0xFF,
		(addr >> 16) & 0xFF,
		(addr >> 8) & 0xFF,
		(addr) & 0xFF
		);
}

int add_record(int src_addr, int payload_size, int dest_addr, double ts){
	int i ;
	int src_addr_found = 0;
	for (i = 0; i < src_num; i++){
		if(src_addrs[i] == src_addr){
			// source already stored
			src_addr_found = 1;
			src_bytecount[i] += payload_size;

			detect_hitter(i, src_addr, src_bytecount[i], detected_flag, ts);
			if (!first_epoch){
				detect_changer(i, pre_src_addrs, pre_src_bytecount, src_addr, src_bytecount, detected_flag, ts);
			}

			int j;
			int dest_addr_found = 0;
			for(j = 0; j < src_dest_num[i]; j++){
				if(dest_addr == src_dest_addrs[i][j]){
					dest_addr_found = 1;
					break;
				}
			}
			if(dest_addr_found == 0){ // dest addr not found
				src_dest_num[i] ++;
				src_dest_addrs[i] = (unsigned int *)realloc(src_dest_addrs[i], sizeof(int)*src_dest_num[i]);
			}

			break;
		}	
	}
	if(src_addr_found){
		return 0;
	}

	// source addrs not found
	src_num++;
	src_addrs = (unsigned int *)realloc(src_addrs, sizeof(int)*src_num);
	src_addrs[src_num-1] = src_addr;
	src_bytecount = (unsigned int *) realloc(src_bytecount, sizeof(int)*src_num);
	src_bytecount[src_num-1] = payload_size;
	src_dest_num = (unsigned int *)realloc(src_dest_num, sizeof(int)*src_num);
	src_dest_num[src_num-1] = 1;
	src_dest_addrs = (unsigned int **) realloc(src_dest_addrs, sizeof(int *) * src_num);
	src_dest_addrs[src_num-1] = (unsigned int *) malloc(sizeof(int)*src_dest_num[src_num-1]);
	src_dest_addrs[src_num-1][src_dest_num[src_num-1]] = dest_addr;

	detected_flag[0] = (char*)realloc(detected_flag, sizeof(char)*src_num);
	detected_flag[0][src_num-1] = 0;
	detected_flag[1] = (char*)realloc(detected_flag, sizeof(char)*src_num);
	detected_flag[1][src_num-1] = 0;
	detected_flag[2] = (char*)realloc(detected_flag, sizeof(char)*src_num);
	detected_flag[2][src_num-1] = 0;

	// detect the first pkt from a src in an epoch
	detect_hitter(0, src_addr, payload_size, detected_flag, ts);
	if (!first_epoch){
		detect_changer(0, pre_src_num, pre_src_addrs, pre_src_bytecount, src_addr, payload_size, detected_flag, ts);
	}
	return 1;
} 

void free_all(){
	int i = 0;
	for (i = 0; i < src_num; i++){
		free(src_dest_addrs[i]);
	}

	free(src_dest_num);
	free(src_bytecount);
	free(src_addrs);
	src_num = 0;
}

// void detect_hitter(unsigned int* srd_addrs, unsigned int* src_bytecount, int hh_thresh){
// 	for (i = 0; i < src_num; i++){
// 		if (src_bytecount[i] > hh_thresh){

// 		}
// 		if(src_addrs[i] == src_addr){
// 			// source already stored
// 			src_addr_found = 1;
// 			src_bytecount[i] += payload_size;
// 			int j;
// 			int dest_addr_found = 0;
// 			for(j = 0; j < src_dest_num[i]; j++){
// 				if(dest_addr == src_dest_addrs[i][j]){
// 					dest_addr_found = 1;
// 					break;
// 				}
// 			}
// 			if(dest_addr_found == 0){ // dest addr not found
// 				src_dest_num[i] ++;
// 				src_dest_addrs[i] = (unsigned int *)realloc(src_dest_addrs[i], sizeof(int)*src_dest_num[i]);
// 			}

// 			break;
// 		}	
// 	}
// }

void detect_hitter(int src_id, unsigned int src_addr, unsigned int src_traffic, char* detected_flag[], double ts){
	enum flag_type flag = hitter;
	if (detected_flag[hitter][src_id] == 1){
		// had been detected this epoch
		return;
	}else if (src_traffic > hh_thresh){
		detected_flag[hitter][src_id] = 1;
		printf("Timestamp: %lf, Type: Heavy Hitter, Src Host: %u\n", ts, src_addr);
	}
}

void detect_changer(int src_id, unsigned int pre_src_sum, unsigned int* pre_src_addrs, unsigned int* pre_src_bytecount, 
	unsigned int cur_src_addr, unsigned int cur_src_traffic, char* detected_flag[], double ts){
	enum flag_type flag = changer;
	if (detected_flag[changer][src_id] = 1){
		return;
	}

	if (pre_src_addrs == NULL && pre_src_bytecount == NULL){
		printf("no pkts previous epoch\n");
	}

	int i = 0, found = 0;
	for (i = 0; i < pre_src_num; i++){
		if (pre_src_addrs[i] == cur_src_addr){
			found = 1;
			if (abs(cur_src_traffic - pre_src_bytecount[i]) > hc_thresh){
				detected_flag[changer][src_id] = 1;
				printf("Timestamp: %lf, Type: Heavy Changer, Src Host: %u\n", ts, cur_src_addr);
			}		
			return;
		}
	}

	// no such case
	// if (!found){
	// 	if (cur_src_traffic - 0 > hc_thresh){
	// 		detected_flag[changer][src_id] = 1;
	// 		printf("Timestamp: %lf, Type: Heavy Changer, Src Host: %u\n", ts, cur_src_addr);
	// 	}
	// }
}

/***************************************************************************
 * Main program
 ***************************************************************************/
int main(int argc, char** argv) {
	pcap_t* pcap;
	char errbuf[256];
	struct pcap_pkthdr hdr;
	const u_char* pkt;					// raw packet
	double pkt_ts;						// raw packet timestamp

	char *filename; // the filename to caputure packet file

	struct ether_header* eth_hdr = NULL;
	struct ip* ip_hdr = NULL;
	struct tcphdr* tcp_hdr = NULL;

	unsigned int src_ip;
	unsigned int dst_ip;
	unsigned short src_port;
	unsigned short dst_port;
	
	// for statistics
	int pkt_num = 0;
	int ipv4_num = 0;
	int ipv4_checksum_num = 0;
	int ip_payload_size = 0;
	int tcp_num = 0;
	int udp_num = 0;
	int icmp_num = 0;


	if (argc != 6) {
		fprintf(stderr, "Arg num error\n");
		exit(-1);
	}

	hh_thresh = atoi(argv[1]); // in MB
	hc_thresh = atoi(argv[2]); // in MB
	ss_thresh = atoi(argv[3]);
	int epoch = atoi(argv[4]); // in msec
	filename = argv[5];

	// open input pcap file                                         
	// if ((pcap = pcap_open_live(filename, 1500, 1, 1000, errbuf)) == NULL) {
	// 	fprintf(stderr, "ERR: cannot open %s (%s)\n", dev, errbuf);
	// 	exit(-1);
	// }

	// read the packets from .pcap file named <filename>
	if ((pcap = pcap_open_offline(filename, errbuf)) == NULL) {
		fprintf(stderr, "ERR: cannot open %s (%s)\n", filename, errbuf);
		exit(-1);
	}

	int first_pkt = 1;
	double last_ts = 0;
	double last_epoch_ts = 0;
	double epoch_time = ((double) epoch) / 1000; // epoch in sec
	double start_time;
	
	while((pkt = pcap_next(pcap, &hdr)) != NULL) {
		// get the timestamp
		pkt_ts = (double)hdr.ts.tv_usec / 1000000 + hdr.ts.tv_sec;

		if(first_pkt){
			// set the timestamp of the first packet
			last_ts = pkt_ts;
			last_epoch_ts = pkt_ts;
			first_pkt = 0;
		}		

		//one packet observed
		pkt_num ++;

		//initial value for the ip_hdr
		ip_hdr = (struct ip*)pkt;

		// // parse the headers

		// eth_hdr = (struct ether_header*)pkt;
		// switch (ntohs(eth_hdr->ether_type)) {
		// 	case ETH_P_IP:		// IP packets (no VLAN header)
		// 		ip_hdr = (struct ip*)(pkt + ETH_HDR_LEN);
		// 		break;
		// 	case 0x8100:		// with VLAN header (with 4 bytes)
		// 		ip_hdr = (struct ip*)(pkt + ETH_HDR_LEN + 4); 
		// 		break;
		// }

		// // if IP header is NULL (not IP or VLAN), continue. 
		// if (ip_hdr == NULL) {
		// 	continue;
		// }

		if(ip_hdr->ip_v == 4){
			// ipv4
			ipv4_num++;
			ip_payload_size += ip_hdr->len - (ip_hdr->ip_hl << 2);
			// may need to use ntoh here
		}else{
			// not ipv4, ignore
			continue;
		}

		if(ip_checksum((unsigned char *)ip_hdr) == ip_hdr->ip_sum){
			//check sum passed
			ipv4_checksum_num++;
		}

		// IP addresses are in network-byte order	
		src_ip = ip_hdr->ip_src.s_addr;
		dst_ip = ip_hdr->ip_dst.s_addr;

		if (ip_hdr->ip_p == IPPROTO_TCP) {
			tcp_hdr = (struct tcphdr*)((u_char*)ip_hdr + 
					(ip_hdr->ip_hl << 2)); 
			src_port = ntohs(tcp_hdr->source);
			dst_port = ntohs(tcp_hdr->dest);

			printf("%lf: %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n", 
					pkt_ts, 
					src_ip & 0xff, (src_ip >> 8) & 0xff, 
					(src_ip >> 16) & 0xff, (src_ip >> 24) & 0xff, 
					src_port, 
					dst_ip & 0xff, (dst_ip >> 8) & 0xff, 
					(dst_ip >> 16) & 0xff, (dst_ip >> 24) & 0xff, 
					dst_port);
			tcp_num++;
		}else if(ip_hdr->ip_p == IPPROTO_UDP){
			udp_num++;

		}else if(ip_hdr->ip_p == IPPROTO_ICMP){
			icmp_num++;

		}

		// test epoch time
		double pass_time = pkt_ts - last_epoch_ts;
		if (pass_time > epoch_time){	// cur packet is in the next epoch, add_record last
			if (first_epoch){
				first_epoch = 0;
			}
			
			detect_spreader();
			// /* changer case:
			//    some srcs in last epoch don't apeare in cur epoch
			//    change is pre_bytecounts->0 
			// */
			// // no such case
			// if (pre_src_addrs != NULL and pre_src_bytecount != NULL){
			// 	for (int i = 0; i < pre_src_num; i++){
			// 		int found = 0;
			// 		for (int j = 0; j < src_num; j++){
			// 			if (pre_src_addrs[i] == src_addrs[j]){
			// 				found = 1;
			// 				break;
			// 			}
			// 		}

			// 		if (!found){
			// 			if (pre_src_bytecount[i] - 0 > hc_thresh){
			// 				printf("Timestamp: ???, Type: Heavy Changer, Src Host: ???\n";
			// 			}
			// 		}
			// 	}
			// }			

			// save cur records as pre records
			if (pre_src_addrs != NULL && pre_src_bytecount != NULL){
				free(pre_src_addrs);
				free(pre_src_bytecount);
				for(int i = 0; i < 3; i++){
					free(detected_flag[i]);
				}
			}
			if (pass_time > 2 * epoch_time){
				// pre epoch is empty
				pre_src_addrs = NULL;
				pre_src_bytecount = NULL;
				pre_src_num = 0;
			}
			else{
				pre_src_addrs = src_addrs;
				pre_src_bytecount = src_bytecount;
				pre_src_num = src_num;
			}

			// cur pkt is the 1st pkt of cur epoch
			src_addrs = NULL;
			src_bytecount = NULL;
			src_num = 0;

			add_record(src_ip, ip_payload_size, dst_ip, pkt_ts);
		}else{
			add_record(src_ip, ip_payload_size, dst_ip, pkt_ts);
		}
	}

	// last_epoch


	// close files
	pcap_close(pcap);

	printf("the total number of observed packets: %d\n", pkt_num);
	printf("the total number of observed IPv4 packets: %d\n", ipv4_num);
	printf("the total number of valid IPv4 packets that pass the checksum test: %d\n", ipv4_checksum_num);
	printf("the total IP payload size: %d\n",ip_payload_size );
	printf("the total number of TCP packets: %d\n", tcp_num);
	printf("the total number of UDP packets: %d\n", udp_num);
	printf("the total number of ICMP packets: %d\n",icmp_num );
	
	return 0;
}
