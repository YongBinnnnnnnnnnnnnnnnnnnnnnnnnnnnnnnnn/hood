#sudo cp 99-fbturbo.conf ~/m2/usr/share/X11/xorg.conf.d/
#sudo cp 99-calibration.conf ~/m2/usr/share/X11/xorg.conf.d/

sudo cp --no-preserve=mode,ownership ~/m2/etc/hosts ./hosts
sudo cp --no-preserve=mode,ownership ~/m2/etc/rc.local ./rc.local
sudo cp --no-preserve=mode,ownership ~/m2/etc/sysctl.conf ./sysctl.conf
sudo cp --no-preserve=mode,ownership ~/m2/etc/danted.conf ./
sudo cp --no-preserve=mode,ownership ~/m2/etc/NetworkManager/dnsmasq.d/dnsmasq.conf ./dnsmasq.conf
sudo cp --no-preserve=mode,ownership ~/m2/etc/NetworkManager/NetworkManager.conf ./NetworkManager.conf
#sudo cp --no-preserve=mode,ownership ~/m2/etc/init.d/firewall ./
sudo cp --no-preserve=mode,ownership ~/m2/etc/systemd/timesyncd.conf ./
sudo cp --no-preserve=mode,ownership ~/m2/usr/lib/systemd/system/before-network.service ./



sudo cp ~/m2/etc/NetworkManager/dispatcher.d/02-my-dispatcher ./
sudo cp ~/m2/etc/NetworkManager/system-connections/* system-connections/
sudo cp ~/m2/var/adguard/* ../../adguard/
sudo cp ~/m2/var/AdGuardHome/AdGuardHome.yaml ./


