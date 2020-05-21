/*
 * sniffer.cc
 * - Use the libpcap library to write a sniffer.
 *   By Patrick P. C. Lee.
 */

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

#define ETH_HDR_LEN 14




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


	if (argc != 5) {
		fprintf(stderr, "Arg num error\n");
		exit(-1);
	}

	int hh_thresh = atoi(argv[1]);
	int hc_thresh = atoi(argv[2]);
	int ss_thresh = atoi(argv[3]);
	int epoch = atoi(argv[4]);
	filename = argv[5];

	// open input pcap file                                         
	// if ((pcap = pcap_open_live(filename, 1500, 1, 1000, errbuf)) == NULL) {
	// 	fprintf(stderr, "ERR: cannot open %s (%s)\n", dev, errbuf);
	// 	exit(-1);
	// }

	// read the packets from .pcap file named <filename>
	if ((pcap = pcap_open_offline(filename, errbuf)) == NULL) {
		fprintf(stderr, "ERR: cannot open %s (%s)\n", dev, errbuf);
		exit(-1);
	}


	while (1) {
		if ((pkt = pcap_next(pcap, &hdr)) != NULL) {
			// get the timestamp
			pkt_ts = (double)hdr.ts.tv_usec / 1000000 + hdr.ts.tv_sec;

			//one packet observed
			pkt_num ++;

			//initial value for the ip_hdr
			ip_hdr = NULL;

			// parse the headers

			eth_hdr = (struct ether_header*)pkt;
			switch (ntohs(eth_hdr->ether_type)) {
				case ETH_P_IP:		// IP packets (no VLAN header)
					ip_hdr = (struct ip*)(pkt + ETH_HDR_LEN); 
					break;
				case 0x8100:		// with VLAN header (with 4 bytes)
					ip_hdr = (struct ip*)(pkt + ETH_HDR_LEN + 4); 
					break;
			}

			// if IP header is NULL (not IP or VLAN), continue. 
			if (ip_hdr == NULL) {
				continue;
			}

			if(ip_hdr->ip_v == 4){
				// ipv4
				ipv4_num++;
				ip_payload_size += ip_hdr->len - (ip_hdr->ip_hl << 2);
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

			}else if(ip_hdr->ip_p == IPPROTO_UDP){
				udp_num++;

			}else if(ip_hdr->ip_p == IPPROTO_ICMP){
				tcp_num++;

			}else{

				// for other than tcp, udp and icmp

			}


		}
	}

	// close files
	pcap_close(pcap);
	
	return 0;
}
