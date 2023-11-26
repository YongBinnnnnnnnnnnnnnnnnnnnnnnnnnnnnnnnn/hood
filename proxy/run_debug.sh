#!/bin/sh

if [ "$1" = "stage1" ]; then
  script_dir=$(readlink -f $(dirname "$0"))
  sudo mount -o bind $script_dir/resolv.conf /etc/resolv.conf
  echo "" > /tmp/hood_proxy/empty_file
  sudo mount -o bind $script_dir/empty_file /etc/hosts
  cat /etc/resolv.conf
  sudo -u $2 $script_dir/build/debug/hood_proxy
  exit
fi

mkdir -p /tmp/hood_proxy
cat /etc/resolv.conf
sudo unshare -m $0 stage1 $(whoami)
