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
disable_wireless=1
disable_gpu=1
target_instrument_set="arm64"
usb_tether=1
wan_port_device_path="auto-built-in-eth"
#rpi4b "/sys/devices/platform/scb/fd580000.ethernet/net/eth0"
#rpi2b "/sys/devices/platform/soc/3f980000.usb/usb1/1-1/1-1.1/1-1.1:1.0/net/eth0"
lodevice=""
yongbin=0
raspberrypi=1
debian_live=0
screen_dtbo="./boot/overlays/joy-IT-Display-Driver-35a-overlay.dtbo"
gpio_shutdown_pin=21

prefix=""
target="/"
for arg in "$@"; do
  case $arg in 
    usb_tether=*) usb_tether=$(echo $arg|sed "s/[^=]*=//");;
    harden_only=*) harden_only=$(echo $arg|sed "s/[^=]*=//");;
    disable_wireless=*) disable_wireless=$(echo $arg|sed "s/[^=]*=//");;
    disable_gpu=*) disable_gpu=$(echo $arg|sed "s/[^=]*=//");;
    gpio_shutdown_pin=*) gpio_shutdown_pin=$(echo $arg|sed "s/[^=]*=//");;
    screen_dtbo=*) screen_dtbo=$(echo $arg|sed "s/[^=]*=//");;
    target=*) target=$(echo $arg|sed "s/[^=]*=//");;
    wan_port_device_path=*) prefix=$(echo $arg|sed "s/[^=]*=//");;
    yongbin) yongbin=1;harden_only=1;;
    debian_live) debian_live=1;raspberrypi=0;harden_only=1;usb_tether=0;disable_wireless=0;disable_gpu=0;;
  esac
done

echo "$@"
echo $harden_only $usb_tether $disable_wireless $prefix $target $target_instrument_set $lodevice

if ! test -d $target; then
  echo "Target is not a directory, try to mount it"
  if ! test -e $target; then
    echo "Target does not exist, try append /dev/ prefix"
    target=/dev/$target
  fi
  if test -f $target; then
    echo "Target is a file, try mount as disk image"
    target=$(sudo losetup --find --show --partscan $target)
    lodevice=$target
    if [ $? -ne 0 ]; then
      echo "losetup failed"
      exit 1
    fi
  fi
  machine=$(uname -s)

  mkdir -p /tmp/hood-install/mnt
  
  if [ $machine = "FreeBSD" ]; then
    sudo lklfuse -o allow_other,type=ext4 ${target}s2 /tmp/hood-install/mnt
    sudo mkdir -p /tmp/hood-install/mnt/boot/firmware
    sudo mount -t msdos ${target}s1 /tmp/hood-install/mnt/boot/firmware
  elif [ $machine = "Linux" ]; then
    case $target in /dev/loop*)
      target=${target}p
    esac
    sudo mount ${target}2 /tmp/hood-install/mnt
    sudo mkdir -p /tmp/hood-install/mnt/boot/firmware
    sudo mount ${target}1 /tmp/hood-install/mnt/boot/firmware
  fi
  if [ $? -ne 0 ]; then
    echo "Failed to mount " $target
    sudo umount /tmp/hood-install/mnt/boot/firmware
    sudo umount /tmp/hood-install/mnt
    exit 1
  fi
  prefix=/tmp/hood-install/mnt
else
  prefix=$target
fi

if file $prefix/usr/bin/ls| grep -q "armhf"; then
  target_instrument_set="armhf"
elif file $prefix/usr/bin/ls| grep -q "x86-64"; then
  target_instrument_set="x64"
fi

sudosedi "s|http://deb.debian.org/|https://deb.debian.org/|g" $prefix/etc/apt/sources.list
sudosedi "s|http://security.debian.org/|https://security.debian.org/|g" $prefix/etc/apt/sources.list

apt_update_done=0
apt_update() {
  if [ $apt_update_done -eq 0 ]; then
    sudo apt update
    apt_update_done=1
  fi
}

apt_install() {
  if ! apt list --installed $1 2>/dev/null|grep -q $1; then
    apt_update
    sudo apt install -y $1
  fi
}

if grep "#" $prefix/etc/ca-certificates.conf; then 
  sed -e '/^$/d' -e '/^#/d' $prefix/etc/ca-certificates.conf | xargs -I {} sh -c "openssl x509 -text -in $prefix/usr/share/ca-certificates/{}|grep -q \"C = US,\"||echo -n \!;echo {}" > /tmp/hood-install/ca-certificates.conf
  sudosedi "s/^!(.*(Comodo|GlobalSign|Baltimore_CyberTrust).*)/\1/ig" /tmp/hood-install/ca-certificates.conf
  sudosedi "s/^[^\!].*(AffirmTrust|Certainly|Starfield|Trustwave|XRamp|emSign).*/\!\0/ig" /tmp/hood-install/ca-certificates.conf
  
  sudocpcontent /tmp/hood-install/ca-certificates.conf $prefix/etc/ca-certificates.conf
  if test -f /usr/sbin/update-ca-certificates; then
    sudo update-ca-certificates --certsconf $prefix/etc/ca-certificates.conf --certsdir $prefix/usr/share/ca-certificates --localcertsdir $prefix/usr/local/share/ca-certificates --etccertsdir $prefix/etc/ssl/certs --hooksdir $prefix/etc/ca-certificates/update.d
  else
    for cert in $(ls $prefix/etc/ssl/certs/*.pem -b -1); do grep "$(realpath $cert|sed -e "s|.*ca-certificates/|\!|")" $prefix/etc/ca-certificates.conf -q && sudo rm -v $cert; done
    sudo openssl rehash $prefix/etc/ssl/certs/
    ls $prefix/etc/ssl/certs/*.pem $prefix/etc/ssl/certs/*.crt -1|grep -v ca-certificates.crt|xargs cat>$prefix/etc/ssl/certs/ca-certificates.crt
  fi
fi
 
sudo mkdir -p $prefix/etc/pki/nssdb
sudocpcontent nssdb/cert9.db $prefix/etc/pki/nssdb
sudocpcontent nssdb/key4.db $prefix/etc/pki/nssdb
sudocpcontent nssdb/pkcs11.txt $prefix/etc/pki/nssdb
sudo chmod 0644 $prefix/etc/pki/nssdb/*

sudo mkdir -p $prefix/etc/skel/.pki/
sudo cp -r $prefix/etc/pki/nssdb $prefix/etc/skel/.pki/
sudo cp ../chromium.sh $prefix/etc/skel/Desktop/

if [ "$prefix" = "" ] || [ "$prefix" = "/" ] ; then
  apt_install dnsmasq
  apt_install network-manager
fi

echo $harden_only $usb_tether $disable_wireless $prefix $target $target_instrument_set $lodevice

if [ $debian_live -eq 0 ] && ! grep -q dtparam $prefix/boot/firmware/config.txt; then
  echo "target location unlikely to be a raspberry pi system mount"
  exit 0
  if ! test -d $target; then
    sudo umount /tmp/hood-install/mnt/boot/firmware
    sudo umount /tmp/hood-install/mnt
  fi
  exit 1
fi

sudo mkdir -p $prefix/usr/local/lib/hood
sudo mkdir -p $prefix/var/lib/hood/flags
echo "${wan_port_device_path}" | sudo tee $prefix/var/lib/hood/wan_port_device_path.txt
sudocpcontent ./ip_subnet_blacklist.txt $prefix/var/lib/hood/
sudocpcontent ./domain_blacklist.txt $prefix/var/lib/hood/
sudocpcontent ./allowed_tls_ports.txt $prefix/var/lib/hood/
sudocpcontent ./expose_to_internal.txt $prefix/var/lib/hood/

if [ $yongbin -eq 1 ]; then
  sudosedi "s/^#//g" $prefix/var/lib/hood/domain_blacklist.txt
  sudosedi "s/^#//g" $prefix/var/lib/hood/expose_to_internal.txt
fi

if [ $harden_only -eq 1 ]; then
  sudo touch $prefix/var/lib/hood/flags/harden_only
elif [ $raspberrypi -eq 1 ]; then
  sudo cp $screen_dtbo $prefix/boot/firmware/overlays/
  if ! grep -q dtoverlay=joy-IT-Display-Driver-35a-overlay $prefix/boot/firmware/config.txt; then
    echo "dtoverlay=$(echo $screen_dtbo|sed -r -s "s|.*/(.*)\.dtbo|\1|"),rotate=90,swapxy=1" | sudo tee -a $prefix/boot/firmware/config.txt 
    echo "dtoverlay=gpio-shutdown,gpio_pin=$gpio_shutdown_pin" | sudo tee -a $prefix/boot/firmware/config.txt 
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
sudo cp hood-expose.py $prefix/usr/local/lib/hood/
sudo chmod 0755 $prefix/usr/local/lib/hood/hood-expose.py
sudocpcontent ./hood_proxy.conf $prefix/etc/

if [ $debian_live -eq 0 ]; then
  if ! grep -q "apparmor" $prefix/boot/firmware/cmdline.txt; then 
    sudosedi "s/ quiet / quiet ipv6.disable=1 apparmor=1 security=apparmor /" $prefix/boot/firmware/cmdline.txt
  fi
elif [ "$prefix" = "" ] || [ "$prefix" = "/" ] ; then
  #not possible
  #if ! grep -q "apparmor" /proc/cmdline; then 
  #  sudosedi "s/ quiet / quiet ipv6.disable=1 apparmor=1 security=apparmor /" /proc/cmdline
  #fi
  true
fi

sudo tee $prefix/etc/modprobe.d/bin-y-blacklist.conf > /dev/null <<EOF
blacklist ipv6
blacklist hci_uart
blacklist i2c_brcmstb
blacklist i2c_dev
EOF

if [ $raspberrypi -eq 1 ] && ! grep -q enable_uart=0 $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
enable_uart=0
EOF
fi

if [ $disable_wireless -eq 1 ]; then
  if ! grep -q disable-bt $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
dtoverlay=disable-bt
dtoverlay=disable-wifi
EOF
  fi
  sudo tee $prefix/etc/modprobe.d/bin-y-disable-wireless-blacklist.conf > /dev/null <<EOF
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
  find $prefix/lib/linux-image*/broadcom -type f|xargs sudo rm
  find $prefix/usr/lib/modules/ -name bluetooth |xargs -I {} find {} -type f|xargs sudo rm
fi

if [ $disable_gpu -eq 1 ]; then
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

  if [ $yongbin -eq 1 ] && ! grep -q framebuffer_width $prefix/boot/firmware/config.txt; then
    sudo tee -a $prefix/boot/firmware/config.txt <<EOF
framebuffer_width=1920
framebuffer_height=1080
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=82
EOF
  fi

  sudo tee $prefix/etc/modprobe.d/bin-y-disable-gpu-blacklist.conf > /dev/null <<EOF
blacklist v3d
blacklist drm
blacklist drm_panel_orientation_quirks
EOF
  find /usr/lib/modules/ -name "gpu" -type d|sudo xargs rm -r
fi


sudo cp ./rc.local $prefix/etc/
sudo chmod +x $prefix/etc/rc.local
sudocpcontent ./hosts $prefix/etc/
sudocpcontent ./sysctl.conf $prefix/etc/
sudocpcontent ./nftables.conf $prefix/etc/

sudo chmod -x  $prefix/etc/*.conf
sudocpcontent ./dhclient.conf $prefix/etc/dhcp/
# This will also diable the hooks of dhclient
echo "deny /{,usr/}bin/bash mr," | sudo tee $prefix/etc/apparmor.d/local/sbin.dhclient > /dev/null
sudocpcontent ./dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo chmod -x  $prefix/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo ln -s /etc/NetworkManager/dnsmasq.d/dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudocpcontent ./NetworkManager.conf $prefix/etc/NetworkManager/NetworkManager.conf
sudo cp firewall $prefix/etc/init.d/
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


sudocpcontent ./before-network.service $prefix/usr/lib/systemd/system/
sudocpcontent ./hood-network-services.service $prefix/usr/lib/systemd/system/
sudo ln -sf /lib/systemd/system/before-network.service $prefix/etc/systemd/system/multi-user.target.wants/before-network.service
sudo ln -sf /lib/systemd/system/hood-network-services.service $prefix/etc/systemd/system/multi-user.target.wants/hood-network-services.service
sudo ln -sf /lib/systemd/system/nftables.service $prefix/etc/systemd/system/sysinit.target.wants/nftables.service
sudo ln -sf /lib/systemd/system/NetworkManager.service $prefix/etc/systemd/system/multi-user.target.wants/NetworkManager.service
sudo ln -sf /lib/systemd/system/NetworkManager-wait-online.service $prefix/etc/systemd/system/network-online.target.wants/NetworkManager-wait-online.service
sudo ln -sf /lib/systemd/system/NetworkManager-dispatcher.service $prefix/etc/systemd/system/dbus-org.freedesktop.nm-dispatcher.service

sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/avahi-daemon.service
sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/cups.path
sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/cups.service
sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/cups-browsed.service
sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/dhcpcd.service
sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/networking.service
sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/connman.service
sudo rm -f $prefix/etc/systemd/system/network-online.target.wants/networking.service
sudo rm -f $prefix/etc/systemd/system/network-online.target.wants/connman-wait-online.service
sudo rm -f $prefix/etc/systemd/system/dbus-org.freedesktop.Avahi.service
sudo rm -f $prefix/etc/systemd/system/dbus-org.freedesktop.timesync1.service
sudo rm -f $prefix/etc/systemd/system/sockets.target.wants/avahi-daemon.socket
sudo rm -f $prefix/etc/systemd/system/ntp.service
sudo rm -f $prefix/etc/systemd/system/ntpd.service
sudo rm -f $prefix/etc/systemd/system/multi-user.target.wants/ntpsec.service
sudo rm -f $prefix/etc/systemd/system/sysinit.target.wants/systemd-timesyncd.service

sudo cp 02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/
sudo ln -sf /etc/NetworkManager/dispatcher.d/02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/pre-up.d/02-hood-dispatcher
sudo ln -sf /etc/NetworkManager/dispatcher.d/02-hood-dispatcher $prefix/etc/NetworkManager/dispatcher.d/pre-down.d/02-hood-dispatcher
sudo chmod 0755 $prefix/etc/NetworkManager/dispatcher.d/02-hood-dispatcher

sudocpcontent 02-hood-firewall-net.rules $prefix/etc/udev/rules.d/

sudo ln -sf /etc/NetworkManager/dnsmasq.d/dnsmasq.conf $prefix/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudo cp update_eeprom $prefix/
sudo chmod +x $prefix/update_eeprom
sudo ln -sf /update_eeprom $prefix/run_once
#sudo touch $prefix/do_init

if [ "$prefix" = "" ] || [ "$prefix" = "/" ] ; then
  if [ $debian_live -eq 1 ]\
    && grep -q "/run/live/medium" /etc/apt/sources.list\
    && grep -q "deb.debian.org" /etc/apt/sources.list\
    && systemctl --version|grep -q 252; then
    #lspci -D |grep hunderbolt | grep "PCI\|NHI" |sed -E "s|(\w+:\w+)([^ ]+).*|\1/\1\2|"|xargs -L 1 -I {} sh -c "echo 1 | sudo tee /sys/devices/pci{}/remove"
    sudo rmmod mei_wdt mei_hdcp mei_me mei
    echo "Debian bookworm live environment found!"
    echo "Install new systemd from backports to enable soft-reboot"
    echo "deb https://deb.debian.org/debian/ bookworm-backports main"|sudo tee -a /etc/apt/sources.list
    sudo apt update
    sudo apt install -y -t bookworm-backports systemd
    sudo rm /etc/network/interfaces
  fi
  mkdir -p ~/.pki/
  cp -r $prefix/etc/pki/nssdb ~/.pki/
  sudocpcontent  /etc/pki/nssdb/cert9.db ~/.pki/nssdb/cert9.db
  ln -s /etc/pki/nssdb/key4.db ~/.pki/nssdb/key4.db
  cp ../chromium.sh ~/Desktop
  sudo systemctl stop ntpd
  if [ $debian_live -eq 1 ]; then
    sudo apt install -y --no-install-recommends git bash-completion chromium chromium-sandbox
    sudo apt autoremove -y --purge avahi-daemon exim4-daemon-light
    sudo systemctl soft-reboot
  fi
  sudo apparmor_parser -r /etc/apparmor.d/sbin.dhclient
  sudo systemctl reload-or-restart before-network nftables NetworkManager NetworkManager-dispatcher hood-network-services
  sudo /etc/rc.local
  echo "Targeting current system, you may need to reboot or soft-reboot to make the firewall fully functional"
fi


if ! test -d $target; then
  sudo umount /tmp/hood-install/mnt/boot/firmware
  sudo umount /tmp/hood-install/mnt
fi

if [ "$lodevice" != "" ]; then
  sudo losetup -d $lodevice
fi
exit 0
