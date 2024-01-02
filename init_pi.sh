#!/bin/sh
script_dir=$(readlink -f $(dirname "$0"))
echo $script_dir
git config --global user.name "Yong Bin"
git config --global user.email yongb@usi.ch

sudo apt -y autoremove --purge bluez bluez-firmware avahi-daemon
# cups
if curl 1.1.1.1; then
  #sudo apt -y install firefox chromium-browser vlc linux-compiler-gcc-12-arm --upgrade 
  sudo apt -y install texlive-font-utils texlive latexmk texlive-fonts-extra texlive-extra-utils
fi

if ! grep -q .hood-proxy /etc/hosts; then
  sudo tee -a /etc/hosts <<EOF
127.0.0.1 example.com
93.184.216.34 example.com.hood-proxy
EOF
fi
cp -Rf $script_dir/../v/home/x/.mozilla ~/

if ! grep -q framebuffer_width /boot/firmware/config.txt; then
  sudo tee -a /boot/firmware/config.txt <<EOF
framebuffer_width=1920
framebuffer_height=1080
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=82
EOF
fi
exit
sudo sed -i "s/dtoverlay=vc4-kms-v3d/dtoverlay=vc4-fkms-v3d/g" /boot/firmware/config.txt
sudo sed -i "s/.*udp dport {6.*//g" /etc/NetworkManager/dispatcher.d/02-hood-dispatcher

#apt-key export 7FA3303E| gpg --dearmor|sudo tee /etc/apt/trusted.gpg.d/raspberrypi-raspbian.gpg|sha256sum |grep e3669c0d6e5a887c668b6c27a57ce47272aeb77373937ffb9939d020c5c16137
#apt-key export 9165938D90FDDD2E| gpg --dearmor|sudo tee /etc/apt/trusted.gpg.d/raspberrypi-raspbian.gpg|sha256sum |grep e3669c0d6e5a887c668b6c27a57ce47272aeb77373937ffb9939d020c5c16137
#sudo sed -i "s|deb \[ arch=armhf \]|deb \[ arch=armhf signed-by=/etc/apt/trusted.gpg.d/raspberrypi-raspbian.gpg \]|g" /etc/apt/sources.list
# avoid Mike Thompson key
sudo tee /etc/apt/sources.list <<EOF
deb [ arch=armhf ] https://archive.raspberrypi.com/debian/ bookworm main rpi
#deb [ arch=armhf ] http://raspbian.raspberrypi.com/raspbian/ bookworm main contrib non-free rpi
#deb [ arch=armhf ] https://deb.debian.org/debian/ bookworm main contrib non-free non-free-firmware
EOF
#cat scripts/apt-trusted-asc/debian-archive-bookworm-automatic.asc|sudo gpg --dearmor -o /etc/apt/trusted.gpg.d/debian-archive-bookworm-automatic.gpg
#sudo gpg --dearmor scripts/apt-trusted-asc/debian-archive-bookworm-automatic.asc -o /etc/apt/trusted.gpg.d/debian-archive-bookworm-security-automatic.gpg
cat scripts/apt-trusted-asc/debian-archive-bookworm-stable.asc|sudo gpg --dearmor -o /etc/apt/trusted.gpg.d/debian-archive-bookworm-stable.gpg 
