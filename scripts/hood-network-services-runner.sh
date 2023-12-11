#!/bin/sh

if [ "$1" = "lo" ]||[ "$1" = "" ]; then
  echo "nameserver 127.0.0.1" > /etc/resolv.conf
  /usr/local/bin/hood-http-handler.py --address 0.0.0.0 2>&1 >> /var/log/hood-http-handler.log &
  /usr/local/bin/hood-name-service.py 2>&1 >> /var/log/hood-name-service.log &
  /usr/local/bin/hood-tls-proxy --config=/etc/hood_proxy.conf 2>&1 >> /var/log/hood-tls-proxy.log &
  /usr/sbin/dnsmasq --keep-in-foreground --no-hosts --no-resolv --no-dhcp --interface=lo --local-service --listen-address=127.0.0.1 --bind-interfaces --no-poll --address=/#/127.0.0.1 --cache-size=0 
else
  mkdir -p /tmp/hood
  touch /tmp/hood/counter.lock
  exec {counter_lock_fd}<>/tmp/hood/counter.lock
  flock $counter_lock_fd
  if ! test -f /tmp/hood/counter; then
    python -c "import secrets; print(2+secrets.randbelow(256-2))" > /tmp/hood/counter
  fi
  counter=$((($(cat /tmp/hood/counter) + 1) % 256))
  if [ $counter -eq 0 ]; then
    $counter=2
  fi
  echo "$counter" > /tmp/hood/counter
  flock -u $counter_lock_fd
  local_address="10.0.${counter}.1"
#  cp /etc/hood_proxy.conf /tmp/hood/
#  sed -i -e -r "s/127.0.0.1//" /etc/hood_proxy.conf
#  /usr/local/bin/hood-http-handler.py --address $ip_local >> /var/log/hood-http-handler.log &
#  /usr/local/bin/hood-tls-proxy --config=/etc/hood_proxy.conf >> /var/log/hood-tls-proxy.log &
  /usr/sbin/dnsmasq --conf-file=/dev/null --no-hosts --no-resolv --keep-in-foreground --interface=$1 --clear-on-reload --strict-order --listen-address=$local_address --bind-interfaces --no-poll --address=/#/$local_address --cache-size=0  --dhcp-range=$local_address,10.0.${counter}.255,infinte &
fi