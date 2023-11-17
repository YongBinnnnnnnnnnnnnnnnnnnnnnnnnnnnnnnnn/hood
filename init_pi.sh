#!/bin/sh
script_dir=$(readlink -f $(dirname "$0"))
echo $script_dir
git config --global user.name "Yong Bin"
git config --global user.email yongb@usi.ch

sudo apt -y autoremove --purge bluez bluez-firmware avahi-daemon
sudo apt -y install firefox chromium-browser vlc linux-compiler-gcc-12-arm --upgrade 

if ! grep -q .hood-proxy /etc/hosts; then
  sudo tee -a /etc/hosts <<EOF
127.0.0.1 example.com
93.184.216.34 example.com.hood-proxy
EOF
fi
cp -Rf $script_dir/../v/home/x/.mozilla ~/
exit

sudo tee /etc/modprobe.d/bin-y-blacklist.conf <<EOF
blacklist bluetooth
EOF

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
