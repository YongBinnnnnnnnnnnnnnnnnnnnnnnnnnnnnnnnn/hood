#!/bin/bash

output=~/hood-$(date "+%Y%m%d%H%M%S").img

source=$(ls ../../*raspios-bookworm-arm64*.xz|sort|tail -n 1)
if echo $1 | grep -q "\.img"; then
  source=$1
fi

case $source in
    *.img.xz) xz -T0 -d -c $source | dd of=$output bs=4M status=progress;;
    *.img) cp $source $output;;
esac

if echo $1 | grep -q "\.img"; then
  ./install.sh "${@:2}" target=$output
else
  ./install.sh "${@}" target=$output
fi

sha256sum $output |sed "s/ .*//" > ${output}.sha256
