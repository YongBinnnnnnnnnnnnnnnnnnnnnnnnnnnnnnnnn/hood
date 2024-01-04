#!/bin/sh

echo "nameserver 127.0.0.1"|sudo tee /etc/resolv.conf
dnsmasq --conf-file=dnsmasq.conf

