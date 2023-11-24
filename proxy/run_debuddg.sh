#!/bin/sh

if [ "$1" = "stag
  exit
fi

mkdir -p /tmp/hood_proxy
cat /etc/resolv.conf
sudo unshare -m $0 stage1
