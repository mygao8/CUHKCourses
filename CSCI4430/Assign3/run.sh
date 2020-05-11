#!/bin/bash
IP="10.3.1.41"      # public interface
LAN="10.0.41.0"   # private LAN network address (without subnet mask)
MASK="24"
bucket_size = "10"
fill_rate = "2"

sudo ./iptables.sh
sudo ./nat ${IP} ${LAN} ${MASK} ${bucket_size} ${fill_rate}
