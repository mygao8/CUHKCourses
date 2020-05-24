#!/bin/bash
#IP="10.3.1.41"      # public interface
IP="10.3.1.41"      # public interface
LAN="10.0.41.0"   # private LAN network address (without subnet mask)
MASK="24"

echo "Public IP = ${IP}, Private LAN = ${LAN}/${MASK}"
echo ""

echo "1" >  /proc/sys/net/ipv4/ip_forward

iptables -t nat -F
iptables -t filter -F
iptables -t mangle -F

