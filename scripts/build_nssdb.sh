#!/bin/sh

cert_conf=/etc/ca-certificates.conf
cert_dir=/usr/share/ca-certificates

if [ $1 ]; then
  cert_conf=$1
fi
if [ $2 ]; then
  cert_dir=$2
fi

rm -rf nssdb
mkdir -p nssdb
certutil -N -d nssdb --empty-password
sed -e '/^$/d' -e '/^[\!#]/d' $cert_conf |xargs -I {} certutil -A -u L -t ',,' -d nssdb -n {} -i $cert_dir/{}