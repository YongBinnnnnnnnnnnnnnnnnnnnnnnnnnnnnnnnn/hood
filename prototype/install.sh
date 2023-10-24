sudo cp -r boot/* ~/m1/
sudo cp 99-fbturbo.conf ~/m2/usr/share/X11/xorg.conf.d/
sudo cp 99-calibration.conf ~/m2/usr/share/X11/xorg.conf.d/

sudo cp --no-preserve=mode,ownership ./modules ~/m2/etc/
sudo cp --no-preserve=mode,ownership ./hosts ~/m2/etc/
sudo cp --no-preserve=mode,ownership ./rc.local ~/m2/etc/
sudo cp --no-preserve=mode,ownership ./sysctl.conf ~/m2/etc/
sudo cp --no-preserve=mode,ownership ./danted.conf ~/m2/etc/
sudo chmod -x  ~/m2/etc/*.conf
sudo cp --no-preserve=mode,ownership ./00-firewall.conf ~/m2/etc/rsyslog.d/
sudo cp --no-preserve=mode,ownership ./dnsmasq.conf ~/m2/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo chmod -x  ~/m2/etc/NetworkManager/dnsmasq.d/dnsmasq.conf
sudo ln -s /etc/NetworkManager/dnsmasq.d/dnsmasq.conf ~/m2/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudo cp --no-preserve=mode,ownership ./NetworkManager.conf ~/m2/etc/NetworkManager/NetworkManager.conf
sudo cp firewall ~/m2/etc/init.d/
sudo cp --no-preserve=mode,ownership ./timesyncd.conf ~/m2/etc/systemd/
sudo chmod -x ~/m2/etc/systemd/timesyncd.conf


#sudo sed -i "s|http://archive.raspberrypi.org/debian/ bullseye main |[arch=arm64] https://mirror.init7.net/raspbian/raspbian/ bullseye main arm64|g" ~/m2/etc/apt/sources.list.d/raspi.list
sudo sed -i "s|http://archive.raspberrypi.org/|https://archive.raspberrypi.org/|g" ~/m2/etc/apt/sources.list.d/raspi.list
#sudo sed -i "s|deb http://deb.debian.org|#deb http://deb.debian.org|g" ~/m2/etc/apt/sources.list
sudo sed -i "s|http://deb.debian.org/debian|https://mirror.init7.net/debian|g" ~/m2/etc/apt/sources.list
sudo sed -i "s|http://security.debian.org/|https://security.debian.org/|g" ~/m2/etc/apt/sources.list

sudo cp --no-preserve=mode,ownership ./before-network.service ~/m2/usr/lib/systemd/system/
systemctl --root=~/m2 enable before-network.service
sudo ln -s /lib/systemd/system/before-network.service ~/m2/etc/systemd/system/multi-user.target.wants/before-network.service
sudo ln -s /lib/systemd/system/NetworkManager.service ~/m2/etc/systemd/system/multi-user.target.wants/NetworkManager.service
sudo ln -s /lib/systemd/system/NetworkManager-wait-online.service ~/m2/etc/systemd/system/network-online.target.wants/NetworkManager-wait-online.service
sudo ln -s /lib/systemd/system/NetworkManager-dispatcher.service ~/m2/etc/systemd/system/dbus-org.freedesktop.nm-dispatcher.service

sudo rm ~/m2/etc/systemd/system/multi-user.target.wants/dhcpcd.service
sudo rm ~/m2/etc/systemd/system/multi-user.target.wants/userconfig.service
sudo rm ~/m2/etc/systemd/system/getty.target.wants/getty@tty1.service
sudo rm ~/m2/etc/systemd/system/dbus-org.freedesktop.Avahi.service

sudo cp system-connections/* ~/m2/etc/NetworkManager/system-connections/
sudo chmod 0600 ~/m2/etc/NetworkManager/system-connections/*

sudo tar -xf ../../AdGuardHome_linux_arm64.tar.gz --directory=$HOME/m2/var/
sudo cp AdGuardHome.yaml  ~/m2/var/AdGuardHome/
sudo cp run_adguard.sh  ~/m2/var/AdGuardHome/
sudo chown -R nobody:nogroup ~/m2/var/AdGuardHome/

sudo cp -rf ../../adguard ~/m2/var/
sudo chmod 777 ~/m2/var/adguard
sudo chmod 666 ~/m2/var/adguard/*


sudo cp login_usi.py ~/m2/var/
sudo chmod 0755 ~/m2/var/login_usi.py 

sudo cp 02-my-dispatcher ~/m2/etc/NetworkManager/dispatcher.d/
sudo ln -s /etc/NetworkManager/dispatcher.d/02-my-dispatcher ~/m2/etc/NetworkManager/dispatcher.d/pre-up.d/02-my-dispatcher
sudo ln -s /etc/NetworkManager/dispatcher.d/02-my-dispatcher ~/m2/etc/NetworkManager/dispatcher.d/pre-down.d/02-my-dispatcher
sudo chmod 0755 ~/m2/etc/NetworkManager/dispatcher.d/02-my-dispatcher

sudo ln -s /etc/NetworkManager/dnsmasq.d/dnsmasq.conf ~/m2/etc/NetworkManager/dnsmasq-shared.d/dnsmasq.conf
sudo cp dante-server_1.4.2+dfsg-7+b2_arm64.deb ~/m2/var/cache/apt/archives/
#sudo touch ~/m2/do_init
