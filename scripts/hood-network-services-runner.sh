#!/bin/sh

if [ "$1" = "lo" ]||[ "$1" = "" ]; then
  echo "nameserver 127.0.0.1" > /etc/resolv.conf
  sudo -u nobody -g nogroup /bin/sh -c "ulimit -S -n 1000000 -s 819200;/usr/local/lib/hood/hood-http-handler.py --address 0.0.0.0"  >> /var/log/hood-http-handler.log 2>&1 &
  sudo -u nobody -g nogroup /bin/sh -c "ulimit -S -n 1000000 -s 819200;/usr/local/lib/hood/hood-name-service.py"  >> /var/log/hood-name-service.log 2>&1 &
  sudo -u nobody -g nogroup /bin/sh -c "ulimit -S -n 1000000 -s 819200;/usr/local/lib/hood/hood-tls-proxy --config=/etc/hood_proxy.conf"  >> /var/log/hood-d-tls-proxy.log 2>&1 &
  /usr/sbin/dnsmasq --keep-in-foreground --no-hosts --no-resolv --no-dhcp --interface=lo --local-service --listen-address=127.0.0.1 --bind-interfaces --no-poll --address=/#/127.0.0.1 --cache-size=0 
fi
