#!/bin/sh
machine=$(uname -s)

mkdir -p /tmp/hood-install

if [ $machine = "FreeBSD" ]; then
  alias sudocpcontent="sudo cp"
  alias sudosedi="sudo sed -i '' -e "
elif [ $machine = "Linux" ]; then
  alias sudocpcontent="sudo cp --no-preserve=mode,ownership"
  alias sudosedi="sudo sed -i -E "
fi

harden_only=0
rfkill=1
gpukill=1
target_instrument_set="arm64"
usb_tether=1
wan_port_device_path="/sys/devices/platform/scb/fd580000.ethernet/net/eth0"

prefix=""
for arg in "$@"; do
  case $arg in 
    no_usb_tether) usb_tether=0;;
    harden_only) harden_only=1;;
    no_rfkill) rfkill=0;;
    no_gpukll) gpukill=0;;
    prefix=*) prefix=$(echo $arg|sed "s/[^=]*=//");;
    wan_port_device_path=*) prefix=$(echo $arg|sed "s/[^=]*=//");;
  esac
done


if file $prefix/usr/bin/ls| grep -q "armhf"; then
  target_instrument_set="armhf"
fi


echo $harden_only $usb_tether $rfkill $prefix $target_instrument_set

if ! grep -q dtparam $prefix/boot/firmware/config.txt; then
  echo "target location unlikely to be a raspberry pi system mount"
  exit 1
fi

sudo mkdir -p $prefix/usr/local/lib/hood
sudo mkdir -p $prefix/var/lib/hood/flags
sudo mkdir -p $prefix/etc/hood/
echo "${wan_port_device_path}" | sudo tee $prefix/var/lib/hood/wan_port_device_path

if [ $harden_only -eq 1 ]; then
  sudo touch $prefix/var/lib/hood/flags/harden_only
else
  cp ./boot/overlays/joy-IT-Display-Driver-35a-overlay.dtbo $prefix/boot/firmware/overlays/
  if ! grep -q dtoverlay=joy-IT-Display-Driver-35a-overlay $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
dtoverlay=joy-IT-Display-Driver-35a-overlay,rotate=90,swapxy=1
EOF
  fi

  sudo rm $prefix/etc/systemd/system/multi-user.target.wants/userconfig.service
  sudo rm $prefix/etc/systemd/system/getty.target.wants/getty@tty1.service

fi

if [ $usb_tether -eq 1 ]; then
  sudo touch $prefix/var/lib/hood/flags/usb_tether
  if ! grep -q dtoverlay=dwc2 $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
dtoverlay=dwc2,dr_mode=peripheral
EOF
  fi
  sudocpcontent ./modules $prefix/etc/

  sudo cp enable-usb-gadget.sh $prefix/usr/local/lib/hood/
  sudo chmod 0755 $prefix/usr/local/lib/hood/enable-usb-gadget.sh
  sudo ln -sf /usr/local/lib/hood/enable-usb-gadget.sh $prefix/usr/local/sbin/
fi

sudo cp hood-http-handler.py $prefix/usr/local/lib/hood/
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-http-handler.py
sudo cp hood-name-service.py $prefix/usr/local/lib/hood/
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-name-service.py
sudo cp hood-resolve.py $prefix/usr/local/lib/hood/
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-resolve.py
sudo ln -sf /usr/local/lib/hood/hood-resolve.py $prefix/usr/local/bin/
sudo cp hood-timesync.py $prefix/usr/local/lib/hood/
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-timesync.py
sudo ln -sf /usr/local/lib/hood/hood-timesync.py $prefix/usr/local/bin/
sudo cp hood-actor.py $prefix/usr/local/lib/hood/
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-actor.py
sudo ln -sf /usr/local/lib/hood/hood-actor.py $prefix/usr/local/bin/
sudo cp hood-network-services-runner.sh $prefix/usr/local/lib/hood/
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-network-services-runner.sh
sudo cp hood_proxy_$target_instrument_set $prefix/usr/local/lib/hood/hood-tls-proxy
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-tls-proxy
sudocpcontent ./hood_proxy.conf $prefix/etc/


if ! grep -q "apparmor" $prefix/boot/firmware/cmdline.txt; then 
  sudosedi "s/ quiet / quiet ipv6.disable=1 apparmor=1 security=apparmor /" $prefix/boot/firmware/cmdline.txt
fi

sudo tee /etc/modprobe.d/bin-y-blacklist.conf > /dev/null <<EOF
blacklist ipv6
blacklist hci_uart
blacklist i2c_brcmstb
blacklist i2c_dev
EOF

if ! grep -q enable_uart=0 $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
enable_uart=0
EOF
fi

if [ $rfkill -eq 1 ]; then
  if ! grep -q disable-bt $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
dtoverlay=disable-bt
dtoverlay=disable-wifi
EOF
  fi
  sudo tee $prefix/etc/modprobe.d/bin-y-rfkill-blacklist.conf > /dev/null <<EOF
blacklist bluetooth
blacklist btbcm
blacklist hci_uart
blacklist i2c_brcmstb
blacklist i2c_dev
blacklist brcmfmac
blacklist brcmutil
blacklist cfg80211
EOF
  sudo rm $prefix/lib/firmware/brcm/*
  find /lib/linux-image*/broadcom -type f|xargs sudo rm
  find $prefix/usr/lib/modules/ -name bluetooth |xargs -I {} find {} -type f|xargs sudo rm
fi

if [ $gpukill -eq 1 ]; then
  if grep -q kms-v3d $prefix/boot/firmware/config.txt; then
    sudosedi "s/dtoverlay=vc4-f?kms-v3d//g" $prefix/boot/firmware/config.txt
  fi
  if test -f $prefix/etc/systemd/system/display-manager.service; then
    sudo rm $prefix/etc/systemd/system/display-manager.service
    echo "GPU is disabled."
    echo "Default lightdm stopped working."
    echo "Please use startx command instead."
    echo "You may also need config.txt to set the resolution of HDMI."
    echo "Example:"
    echo "framebuffer_width=1920"
    echo "framebuffer_height=1080"
    echo "hdmi_force_hotplug=1"
    echo "hdmi_group=2"
    echo "hdmi_mode=82"
  fi
  sudo tee $prefix/etc/modprobe.d/bin-y-rfkill-blacklist.conf > /dev/null <<EOF
blacklist v3d
EOF
fi

sudocpcontent ./rc.local $prefix/etc/
sudocpcontent ./hosts $prefix/etc/
sudocpcontent ./sysctl.conf $prefix/etc/
sudocpcontent ./nftables.conf $prefix/etc/

if grep "#" $prefix/etc/ca-certificates.conf; then 
  sed -e '/^$/d' -e '/^#/d' $prefix/etc/ca-certificates.conf | xargs -I {} sh -c "openssl x509 -text -in $prefix/usr/share/ca-certificates/{}|grep -q \"C = US,\"||echo -n \!;echo {}" > /tmp/hood-install/ca-certificates.conf
  sudosedi "s/^!(.*(Comodo|GlobalSign|Baltimore_CyberTrust).*)/\1/ig" /tmp/hood-install/ca-certificates.conf
  sudosedi "s/^[^\!].*(AffirmTrust|Trustwave).*/\!\0/ig" /tmp/hood-install/ca-certificates.conf
  
  sudocpcontent /tmp/hood-install/ca-certificates.conf $prefix/etc/ca-certificates.conf
  if which update-ca-certificates; then
    sudo update-ca-certificates --certsconf $prefix/etc/ca-certificates.conf --certsdir $prefix/usr/share/ca-certificates --localcertsdir $prefix/usr/local/share/ca-certificates --etccertsdir $prefix/etc/ssl/certs --hooksdir $prefix/etc/ca-certificates/update.d
  else
    echo "TODO"
  fi
fi
 
sudo mkdir -p $prefix/etc/pki/nssdb
sudocpcontent nssdb/cert9.db $prefix/etc/pki/nssdb
sudocpcontent nssdb/key4.db $prefix/etc/pki/nssdb
sudocpcontent nssdb/pkcs11.txt $prefix/etc/pki/nssdb
sudo chmod 0644 /etc/pki/nssdb/*

sudo mkdir -p $prefix/etc/skel/.pki/
sudo cp -r $prefix/etc/pki/nssdb $prefix/etc/skel/.pki/
if [ "$prefix" = "" ] || [ "$prefix" = "/" ] ; then
  mkdir -p ~/.pki/
  cp -r $prefix/etc/pki/nssdb ~/.pki/
  sudocpcontent  /etc/pki/nssdb/cert9.db ~/.pki/nssdb/cert9.db
  ln -s /etc/pki/nssdb/cert9.db ~/.pki/nssdb/key4.db
fi

sudo chmod -x  $prefix/etc/*.conf
sudocpcontent ./dhclient.conf $prefix/etc/dhcp/
# This will also diable the hooks of dhclient
echo "deny /{,usr/}bin/bash mr," | sudo tee $prefix/etc/apparmor.d/local/sbin.dhclient > /dev/null
sudocpcontent ./dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo chmod -x  $prefix/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo ln -s /etc/NetworkManager/dnsmasq.d/dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudocpcontent ./NetworkManager.conf $prefix/etc/NetworkManager/NetworkManager.conf
sudo cp firewall $prefix/etc/init.d/
sudocpcontent ./timesyncd.conf $prefix/etc/systemd/
sudo chmod -x $prefix/etc/systemd/timesyncd.conf
#https://mirrors.ircam.fr/pub/raspbian/raspbian
#https://mirror.ps.kz/raspbian/raspbian
#https://goddess-gate.com/archive.raspberrypi.com/raspbian
#https://ftp.crifo.org/raspbian/raspbian/
#https://mirrors.gigenet.com/raspbian/raspbian/
#https://mirrors.ocf.berkeley.edu/raspbian/raspbian/
#https://mirrors.ocf.berkeley.edu/raspbian/raspbian/
#https://mirrors.ustc.edu.cn/archive.raspberrypi.org/debian/
#https://mirror01.ikoula.com/raspbian/mirror/archive.raspberrypi.org/debian/
sudosedi "s|http://archive.raspberrypi..../debian/|https://mirrors.tuna.tsinghua.edu.cn/raspberrypi/|g" $prefix/etc/apt/sources.list.d/raspi.list
#sudosedi "s|http://archive.raspberrypi..../|https://archive.raspberrypi.com/|g" $prefix/etc/apt/sources.list.d/raspi.list
#sudosedi "s|deb http://deb.debian.org|#deb http://deb.debian.org|g" $prefix/etc/apt/sources.list
# never use http mirror and lets encrypt https mirror
sudosedi "s|http://raspbian.raspberrypi.com/raspbian/|https://mirror.bardia.tech/raspbian/raspbian/|g" $prefix/etc/apt/sources.list
# when using apt command, the connection to http://raspbian.raspberrypi.com redirected to https://mirror.init7.net for suspicious unknown reason
# use less Mike Thompson key
sudosedi "s|contrib non-free||g" $prefix/etc/apt/sources.list
# todo: find a safer source, both init7 and rasp are not safe
# what can I do if the signing key of the apt source no more trustworthy?

sudosedi "s|http://deb.debian.org/debian|https://deb.debian.org/debian|g" $prefix/etc/apt/sources.list
sudosedi "s|http://security.debian.org/|https://security.debian.org/|g" $prefix/etc/apt/sources.list

sudocpcontent ./before-network.service $prefix/usr/lib/systemd/system/
sudocpcontent ./hood-network-services.service $prefix/usr/lib/systemd/system/
sudo ln -sf /lib/systemd/system/before-network.service $prefix/etc/systemd/system/multi-user.target.wants/before-network.service
sudo ln -sf /lib/systemd/system/hood-network-services.service $prefix/etc/systemd/system/multi-user.target.wants/hood-network-services.service
sudo ln -sf /lib/systemd/system/nftables.service $prefix/etc/systemd/system/sysinit.target.wants/nftables.service
sudo ln -sf /lib/systemd/system/NetworkManager.service $prefix/etc/systemd/system/multi-user.target.wants/NetworkManager.service
sudo ln -sf /lib/systemd/system/NetworkManager-wait-online.service $prefix/etc/systemd/system/network-online.target.wants/NetworkManager-wait-online.service
sudo ln -sf /lib/systemd/system/NetworkManager-dispatcher.service $prefix/etc/systemd/system/dbus-org.freedesktop.nm-dispatcher.service

if test -f $prefix/etc/systemd/system/sysinit.target.wants/systemd-timesyncd.service; then
  sudo rm $prefix/etc/systemd/system/multi-user.target.wants/avahi-daemon.service
  sudo rm $prefix/etc/systemd/system/multi-user.target.wants/cups.path
  sudo rm $prefix/etc/systemd/system/multi-user.target.wants/cups.service
  sudo rm $prefix/etc/systemd/system/multi-user.target.wants/cups-browsed.service
  sudo rm $prefix/etc/systemd/system/multi-user.target.wants/dhcpcd.service
  sudo rm $prefix/etc/systemd/system/dbus-org.freedesktop.Avahi.service
  sudo rm $prefix/etc/systemd/system/dbus-org.freedesktop.timesync1.service
  sudo rm $prefix/etc/systemd/system/sysinit.target.wants/systemd-timesyncd.service
fi

sudo cp 02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/
sudo ln -sf /etc/NetworkManager/dispatcher.d/02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/pre-up.d/02-hood-dispatcher
sudo ln -sf /etc/NetworkManager/dispatcher.d/02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/pre-down.d/02-hood-dispatcher
sudo chmod 0755 $prefix/etc/NetworkManager/dispatcher.d/02-hood-dispatcher

sudo ln -sf /etc/NetworkManager/dnsmasq.d/dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudo cp update_eeprom $prefix/
sudo chmod +x $prefix/update_eeprom
sudo ln -sf /update_eeprom $prefix/run_once
#sudo touch $prefix/do_init
