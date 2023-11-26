#!/bin/sh

if [ "$1" = "stage1" ]; then
  script_dir=$(readlink -f $(dirname "$0"))
  sudo mount -o bind $script_dir/proxy/resolv.conf /etc/resolv.conf
  cat /etc/resolv.conf
  sudo -u $2 ./code.sh --proxy-server=socks5://127.0.2.1:11371
  exit
fi

cat /etc/resolv.conf
sudo unshare -m $0 stage1 $(whoami)
