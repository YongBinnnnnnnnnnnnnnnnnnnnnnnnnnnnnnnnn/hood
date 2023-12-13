#!/bin/sh

if [ "$1" = "lo" ]||[ "$1" = "" ]; then
  echo "nameserver 127.0.0.1" > /etc/resolv.conf
  /usr/local/bin/hood-http-handler.py --address 0.0.0.0 2>&1 >> /var/log/hood-http-handler.log &
  /usr/local/bin/hood-name-service.py 2>&1 >> /var/log/hood-name-service.log &
  /usr/local/bin/hood-tls-proxy --config=/etc/hood_proxy.conf 2>&1 >> /var/log/hood-tls-proxy.log &
  /usr/sbin/dnsmasq --keep-in-foreground --no-hosts --no-resolv --no-dhcp --interface=lo --local-service --listen-address=127.0.0.1 --bind-interfaces --no-poll --address=/#/127.0.0.1 --cache-size=0 
fi