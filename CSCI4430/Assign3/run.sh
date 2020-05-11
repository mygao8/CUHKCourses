#!/bin/bash
IP="10.3.1.41"      # public interface
LAN="10.0.41.0"   # private LAN network address (without subnet mask)
MASK="24"
BUCKET_SIZE="10"
FILL_RATE="2"

sudo ./iptables.sh
sudo ./nat ${IP} ${LAN} ${MASK} ${BUCKET_SIZE} ${FILL_RATE}
