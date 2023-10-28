#!/bin/sh
export SOCKS5_PROXY="10.0.2.1:11371"
git config --global http.proxy 'socks5h://10.0.2.1:11371'
#sudo -E pkg -c $(dirname "$0")/pkg_environment/ install texlive-full vscode
script_dir=$(readlink -f $(dirname "$0"))
echo $script_dir
sudo service avahi-daemon stop
sudo service ntpd stop
sudo devctl disable -f smbios0
sudo devctl disable -f ichsmb0
sudo devctl disable -f acpi_wmi0
sudo devctl disable -f acpi_wmi1
sudo devctl disable -f acpi_wmi2
sudo devctl disable -f fpupnp0
sudo devctl disable -f pcib1
sudo devctl disable -f pcib2
sudo devctl disable -f pcib3
sudo devctl disable -f atdma0
sudo devctl disable -f atrtc0
sudo devctl disable -f efirtc0
sudo devctl disable -f uart0
sudo devctl disable -f uart1
sudo devctl disable -f uart2
sudo devctl disable -f em0
sudo kldunload acpi_wmi ichsmb mac_ntpd
sudo sysrc pf_enable=yes
sudo sysrc pflog_enable=yes
sudo tee /etc/pf.conf <<EOF
set skip on lo0
set block-policy drop
block in quick inet6 all
block out quick inet6 all
pass out quick proto tcp to any port {11371, 443, 80, 53} keep state
pass out log quick proto udp to any port {53} keep state
pass out log quick proto udp from any port 67 to any port 68 keep state
block log  quick all
EOF
sudo tee /etc/dhclient.conf <<EOF
interface "em0" {
  send dhcp-client-identifier "$(LC_ALL=C tr -dc "a-zA-Z0-9\-"</dev/urandom|head -c $(jot -r 1 1 15))";
  send host-name "$(LC_ALL=C tr -dc "a-zA-Z0-9\-"</dev/urandom|head -c $(jot -r 1 1 15))";
}
interface "ue0" {
  send dhcp-client-identifier "$(LC_ALL=C tr -dc "a-zA-Z0-9\-"</dev/urandom|head -c $(jot -r 1 1 15))";
  send host-name "$(LC_ALL=C tr -dc "a-zA-Z0-9\-"</dev/urandom|head -c $(jot -r 1 1 15))";
}
EOF
sudo tee -a /etc/hosts <<EOF
127.0.0.1 livecd
#0.0.0.0 firefox.settings.services.mozilla.com
#0.0.0.0 contile.services.mozilla.com
#0.0.0.0 shavar.services.mozilla.com
#0.0.0.0 content-signature-2.cdn.mozilla.net
#0.0.0.0 firefox-settings-attachments.cdn.mozilla.net
0.0.0.0 safebrowsing.googleapis.com
0.0.0.0 redirector.gvt1.com
0.0.0.0 openvsxorg.blob.core.windows.net
EOF

if ! grep socks5 /usr/local/etc/pkg.conf; then
sudo tee -a /usr/local/etc/pkg.conf <<EOF
pkg_env: {
  socks5_proxy: "10.0.2.1:11371"
}
EOF
fi

prefs_js="$(find ~/.mozilla/firefox/|grep prefs.js)"
if ! grep socks_port $prefs_js; then
tee -a $prefs_js<<EOF
user_pref("network.proxy.socks", "10.0.2.1");
user_pref("network.proxy.socks_port", 11371);
user_pref("network.proxy.type", 1);
EOF
fi
exit

if ! ifconfig em0|grep active; then
  sudo ifconfig em0 link random
  cd /tmp
  mate-terminal -e "sudo tcpdump -n -e -ttt -i pflog0"
  cd -
fi
if ! ifconfig ue0|grep active; then
  sudo ifconfig ue0 link random
  mate-terminal -e "sudo tcpdump -n -e -ttt -i pflog0"
fi
sudo service pf start
sudo service pflog start
sudo pfctl -f /etc/pf.conf
cp -Rf $script_dir/../v/home/x/.mozilla ~/
git config --global user.name "Yong Bin"
git config --global user.email yongb@usi.ch
#sudo mkdir -p /usr/local/texlive/
sudo rm /usr/local/share/texmf-dist

sudo dhclient ue0
if ! [ -f /tmp/ca-root-nss.crt ]; then
  certctl -v list|grep "subject=C = "|grep -v "=C = US"|awk '$2="/etc/ssl/certs/"{print $2$1}'|xargs readlink -f|xargs -I {} sudo mv {} /tmp/
  sudo mv /usr/local/share/certs/ca-root-nss.crt  /tmp/
  tr '\n' '\0' < /tmp/ca-root-nss.crt|sed -re "s/Certificate\:\x00/\n/g"|sed "s/.*C = [^U][^S].*//g"|tr "\n" "%"|sed -E "s/%+/Certificate:\x00/g"|tr '\0' '\n'|sudo tee /usr/local/share/certs/ca-root-nss.crt
  sudo certctl rehash
fi

if sudo -E pkg upgrade -y firefox; then
  sudo -E pkg install -y texlive-base vscode cmake-core
  sudo mv /usr/local/share/texmf-dist /usr/local/share/texmf-dist-old
  sudo ln -s $script_dir/environment/usr/local/share/texmf-dist /usr/local/share/texmf-dist
  sudo -E pkg -o RUN_SCRIPTS=false -o INSTALL_AS_USER=true add download/fake-texmf.tar
  sudo -E pkg install -y latex-mk
  sudo -E pkg -o INSTALL_AS_USER=true install -y tex-dvipsk
fi
