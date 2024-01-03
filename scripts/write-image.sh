#!/bin/bash

echo Writing
sudo dd if=$1 of=$2 bs=4M status=progress
echo Verifying 
image_size=$(stat -c "%s" $1)
count=$(($image_size / 1024 / 1024 / 4))
echo Hash of new disk:
sudo dd if=$2 bs=4M count=$count|sha256sum 
if test -f $1.sha256; then
  echo Hash of image:
  cat $1.sha256
fi
