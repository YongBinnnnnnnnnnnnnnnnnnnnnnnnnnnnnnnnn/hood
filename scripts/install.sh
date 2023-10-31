#!/bin/sh
machine=$(uname -s)

if [ $machine = "FreeBSD" ]; then
  alias sudocpcontent="sudo cp"
  alias sudosedi="sudo sed -i '' -e "
elif [ $machine = "Linux" ]; then
  alias sudocpcontent="sudo cp --no-preserve=mode,ownership"
  alias sudosedi="sudo sed -i "
fi

harden_only=0
prefix=""
for arg in "$@"; do
  case $arg in 
    harden_only) harden_only=1;;
    prefix=*) prefix=$(echo $arg|sed "s/.*=//g");;
  esac
done

echo $harden_only $prefix
#exit

if [ $harden_only -eq 1 ]; then
  sudo touch $prefix/var/hood_harden_only
  if ! grep disable-bt $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
dtoverlay=disable-bt
dtoverlay=disable-wifi
EOF
  fi
else
  sudo cp -r boot/* $prefix/boot/firmware/
  sudocpcontent ./modules $prefix/etc/
  sudocpcontent ./rc.local $prefix/etc/
  sudocpcontent ./danted.conf $prefix/etc/
  sudocpcontent ./00-firewall.conf $prefix/etc/rsyslog.d/

  sudo rm $prefix/etc/systemd/system/multi-user.target.wants/userconfig.service
  sudo rm $prefix/etc/systemd/system/getty.target.wants/getty@tty1.service

  sudo cp system-connections/* $prefix/etc/NetworkManager/system-connections/
  sudo chmod 0600 $prefix/etc/NetworkManager/system-connections/*

  sudo tar -xf ../../AdGuardHome_linux_arm64.tar.gz --directory=$HOME/m2/var/
  sudo cp AdGuardHome.yaml  $prefix/var/AdGuardHome/
  sudo cp run_adguard.sh  $prefix/var/AdGuardHome/
  sudo chown -R nobody:nogroup $prefix/var/AdGuardHome/

  sudo cp -rf ../../adguard $prefix/var/
  sudo chmod 777 $prefix/var/adguard
  sudo chmod 666 $prefix/var/adguard/*

  sudo cp dante-server_1.4.2+dfsg-7+b2_arm64.deb $prefix/var/cache/apt/archives/
fi

sudo cp 99-fbturbo.conf $prefix/usr/share/X11/xorg.conf.d/
sudo cp 99-calibration.conf $prefix/usr/share/X11/xorg.conf.d/

sudocpcontent ./hosts $prefix/etc/
sudocpcontent ./sysctl.conf $prefix/etc/
sudocpcontent ./nftables.conf $prefix/etc/
sudo chmod -x  $prefix/etc/*.conf
sudocpcontent ./dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo chmod -x  $prefix/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo ln -s /etc/NetworkManager/dnsmasq.d/dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudocpcontent ./NetworkManager.conf $prefix/etc/NetworkManager/NetworkManager.conf
sudo cp firewall $prefix/etc/init.d/
sudocpcontent ./timesyncd.conf $prefix/etc/systemd/
sudo chmod -x $prefix/etc/systemd/timesyncd.conf


#sudosedi "s|http://archive.raspberrypi.org/debian/ bullseye main |[arch=arm64] https://mirror.init7.net/raspbian/raspbian/ bullseye main arm64|g" $prefix/etc/apt/sources.list.d/raspi.list
sudosedi "s|http://archive.raspberrypi.org/|https://archive.raspberrypi.org/|g" $prefix/etc/apt/sources.list.d/raspi.list
#sudosedi "s|deb http://deb.debian.org|#deb http://deb.debian.org|g" $prefix/etc/apt/sources.list
sudosedi "" -e "s|http://deb.debian.org/debian|https://mirror.init7.net/debian|g" $prefix/etc/apt/sources.list
sudosedi "s|http://security.debian.org/|https://security.debian.org/|g" $prefix/etc/apt/sources.list

sudocpcontent ./before-network.service $prefix/usr/lib/systemd/system/
systemctl --root=$prefix enable before-network.service
sudo ln -s /lib/systemd/system/before-network.service $prefix/etc/systemd/system/multi-user.target.wants/before-network.service
sudo ln -s /lib/systemd/system/nftables.service $prefix/etc/systemd/system/sysinit/nftables.service
sudo ln -s /lib/systemd/system/NetworkManager.service $prefix/etc/systemd/system/multi-user.target.wants/NetworkManager.service
sudo ln -s /lib/systemd/system/NetworkManager-wait-online.service $prefix/etc/systemd/system/network-online.target.wants/NetworkManager-wait-online.service
sudo ln -s /lib/systemd/system/NetworkManager-dispatcher.service $prefix/etc/systemd/system/dbus-org.freedesktop.nm-dispatcher.service

sudo rm $prefix/etc/systemd/system/multi-user.target.wants/dhcpcd.service
sudo rm $prefix/etc/systemd/system/dbus-org.freedesktop.Avahi.service

sudo cp 02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/
sudo ln -s /etc/NetworkManager/dispatcher.d/02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/pre-up.d/02-hood-dispatcher
sudo ln -s /etc/NetworkManager/dispatcher.d/02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/pre-down.d/02-hood-dispatcher
sudo chmod 0755 $prefix/etc/NetworkManager/dispatcher.d/02-hood-dispatcher

sudo ln -s /etc/NetworkManager/dnsmasq.d/dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudo cp update_eeprom $prefix/
sudo chmod +x $prefix/update_eeprom
sudo ln -s /update_eeprom $prefix/run_once
#sudo touch $prefix/do_init
