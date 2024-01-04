#!/bin/sh
if ! [ "$1" ]; then
  cp umount_all.sh /tmp/
  cd /tmp
  mate-terminal -e "/tmp/umount_all.sh 1"
  exit
fi
while lsof |grep  "/dev/da"; do
  sudo kill $(lsof |grep  "/dev/da"|head -n 1|awk '{print $2}')
done
sudo umount /dev/da*
