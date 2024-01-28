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

  while read line || [ -n "$line" ]; do
    case $line in \#*)
      continue
    esac
    if [ "$line" = "" ]; then
      continue
    fi
    argc="$(echo $line|sed "s/[^ ]//g")"
    if [ "$argc" = " " ]; then
      port=$(echo $line|cut -d " " -f1)
      address=$(echo $line|cut -d " " -f2)
    else
      echo invalid expose to internal format: $line
      continue
    fi
    sudo -u nobody -g nogroup /bin/sh -c "ulimit -S -n 1000000;ulimit -S -s 819200;/usr/local/lib/hood/hood-expose.py --from-address=$address --from-port=$port --to-address=0.0.0.0 --to-port=$port" >> /var/log/hood-expose.log 2>&1 &
  done < /var/lib/hood/expose_to_internal.txt

  tail -f /var/log/hood-name-service.log |grep --line-buffered "\->" |sed "s/.*\-> b/DNS query /" > $log_device &
  #TODO: log discard
  tail -f /var/log/hood-tls-proxy.log |grep --line-buffered "verifying\|handshake failed\|DoConnectHost"|sed -e "s/^[^.]*\.cpp:[0-9]* operator... //" -e "s/:443 :/ :/" -e "s/^[^.]*\.cpp:[0-9]* DoConnectHost\: //" > $log_device &
  /usr/sbin/dnsmasq --keep-in-foreground --no-hosts --no-resolv --no-dhcp --interface=lo --local-service --listen-address=127.0.0.1 --bind-interfaces --no-poll --address=/#/127.0.0.1 --cache-size=0 
fi
