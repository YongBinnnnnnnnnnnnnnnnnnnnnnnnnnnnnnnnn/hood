#!/bin/sh
#sudo pkg -c $(dirname "$0")/pkg_environment/ install texlive-full vscode
script_dir=$(readlink -f $(dirname "$0"))
echo $script_dir
sudo service avahi-daemon stop
sudo sysrc pf_enable=yes
sudo sysrc pflog_enable=yes
sudo tee /etc/pf.conf <<EOF
set skip on lo0
block in quick inet6 all
block out quick inet6 all
pass out quick proto tcp to any port {443, 80, 53} keep state
pass out log quick proto udp to any port {53} keep state
pass out log quick proto udp from any port 67 to any port 68 keep state
block log quick all
EOF
sudo service pf start
sudo service pflog start
sudo pfctl -f /etc/pf.conf
sudo cp -pf $script_dir/../dhclient.conf /etc/
cp -Rf $script_dir/../v/home/x/.mozilla ~/
git config --global user.name "Yong Bin"
git config --global user.email yongb@usi.ch
#sudo mkdir -p /usr/local/texlive/
sudo rm /usr/local/share/texmf-dist
sudo pkg install -y texlive-base vscode
sudo rm -rf /usr/local/share/texmf-dist
sudo ln -s $script_dir/environment/usr/local/share/texmf-dist /usr/local/share/texmf-dist

mate-terminal -e "sudo tcpdump -n -e -ttt -i pflog0"
