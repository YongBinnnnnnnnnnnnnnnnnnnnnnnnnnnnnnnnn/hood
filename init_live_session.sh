#!/bin/sh
#sudo pkg -c $(dirname "$0")/pkg_environment/ install texlive-full vscode
script_dir=$(readlink -f $(dirname "$0"))
echo $script_dir
sudo service avahi-daemon stop
sudo service ntpd stop
sudo devctl disable -f smbios0
sudo devctl disable -f ichsmb0
sudo devctl disable -f acpi_wmi0
sudo devctl disable -f acpi_wmi1
sudo devctl disable -f fpupnp0
sudo devctl disable -f pcib1
sudo devctl disable -f pcib2
sudo devctl disable -f pcib3
sudo devctl disable -f atdma0
sudo devctl disable -f efirtc0
sudo kldunload acpi_wmi ichsmb mac_ntpd
sudo sysrc pf_enable=yes
sudo sysrc pflog_enable=yes
sudo tee /etc/pf.conf <<EOF
set skip on lo0
set block-policy drop
block in quick inet6 all
block out quick inet6 all
pass out quick proto tcp to any port {443, 80, 53} keep state
pass out log quick proto udp to any port {53} keep state
pass out log quick proto udp from any port 67 to any port 68 keep state
block log  quick all
EOF
sudo tee /etc/dhclient.conf <<EOF
interface "em0" {
  send host-name "$(LC_ALL=C tr -dc "a-zA-Z0-9\-_ "</dev/urandom|head -c $(jot -r 1 5 13))";
}
interface "ue0" {
  send host-name "$(LC_ALL=C tr -dc "a-zA-Z0-9\-_ "</dev/urandom|head -c $(jot -r 1 5 13))";
}
EOF
echo "127.0.0.1 livecd"|sudo tee -a /etc/hosts
if ! ifconfig em0|grep active; then
  sudo ifconfig em0 link random
fi
sudo ifconfig em0 down
if ! ifconfig ue0|grep active; then
  sudo ifconfig ue0 link random
fi
sudo service pf start
sudo service pflog start
sudo pfctl -f /etc/pf.conf
sudo cp -pf $script_dir/../dhclient.conf /etc/
cp -Rf $script_dir/../v/home/x/.mozilla ~/
git config --global user.name "Yong Bin"
git config --global user.email yongb@usi.ch
#sudo mkdir -p /usr/local/texlive/
sudo rm /usr/local/share/texmf-dist

if sudo pkg upgrade -y firefox; then
  sudo pkg install -y texlive-base vscode
  sudo mv /usr/local/share/texmf-dist /usr/local/share/texmf-dist-old
  sudo ln -s $script_dir/environment/usr/local/share/texmf-dist /usr/local/share/texmf-dist
  sudo pkg -o RUN_SCRIPTS=false -o INSTALL_AS_USER=true add download/fake-texmf.tar
  sudo pkg install -y tex-basic-engines
  sudo pkg -o INSTALL_AS_USER=true install -y tex-dvipsk
fi
