#!/bin/bash
IP="10.3.1.41"      # public interface
LAN="10.0.41.0"   # private LAN network address (without subnet mask)
MASK="24"
BUCKET_SIZE="2"
FILL_RATE="1"

sudo ./iptables.sh
sudo ./nat ${IP} ${LAN} ${MASK} ${BUCKET_SIZE} ${FILL_RATE}
