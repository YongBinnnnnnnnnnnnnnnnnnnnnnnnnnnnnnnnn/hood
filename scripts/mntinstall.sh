mkdir ~/m1 ~/m2
sudo mount /dev/sdb1 ~/m1
sudo mount /dev/sdb2 ~/m2
./install.sh
sudo umount ~/m1 ~/m2
