#!/bin/bash
IP="10.3.1.41"      # public interface
LAN="10.0.41.0"   # private LAN network address (without subnet mask)
MASK="24"
BUCKET_SIZE="2"
FILL_RATE="1"

sudo ./iptables.sh
<<<<<<< HEAD
sudo ./nat ${IP} ${LAN} ${MASK} ${BUCKET_SIZE} ${FILL_RATE}
 
=======
sudo ./nat ${IP} ${LAN} ${MASK} ${BUCKET_SIZE} ${FILL_RATE}
>>>>>>> 85ff061386f3a50bb6c032869293a7e43f925640
