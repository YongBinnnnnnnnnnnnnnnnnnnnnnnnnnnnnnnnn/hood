#!/bin/bash

log_device=/dev/tty1
if test -f /var/lib/hood/flags/harden_only; then
  log_device=/dev/tty8
fi

if [ "$1" = "lo" ]||[ "$1" = "" ]; then
  echo "nameserver 127.0.0.1" > /etc/resolv.conf
  sudo -u nobody -g nogroup /bin/sh -c "ulimit -S -n 1000000;ulimit -S -s 819200;/usr/local/lib/hood/hood-http-handler.py --address 0.0.0.0" >> /var/log/hood-http-handler.log 2>&1 &
  sudo -u nobody -g nogroup /bin/sh -c "ulimit -S -n 1000000;ulimit -S -s 819200;/usr/local/lib/hood/hood-name-service.py" >> /var/log/hood-name-service.log 2>&1 &
  sudo -u nobody -g nogroup /bin/sh -c "ulimit -S -n 1000000;ulimit -S -s 819200;/usr/local/lib/hood/hood-tls-proxy --config=/etc/hood_proxy.conf" >> /var/log/hood-tls-proxy.log 2>&1 &
  
  tail -f /var/log/hood-name-service.log |grep --line-buffered "\->" |sed "s/.*\-> b/DNS query /" > $log_device &
  tail -f /var/log/hood-tls-proxy.log |grep --line-buffered "verifying\|DoConnectHost"|sed -e "s/^[^.]*\.cpp:[0-9]* operator...  //" -e "s/:443 :/ :/" -e "s/^[^.]*\.cpp:[0-9]* DoConnectHost\: //" > $log_device &
  /usr/sbin/dnsmasq --keep-in-foreground --no-hosts --no-resolv --no-dhcp --interface=lo --local-service --listen-address=127.0.0.1 --bind-interfaces --no-poll --address=/#/127.0.0.1 --cache-size=0 
fi
