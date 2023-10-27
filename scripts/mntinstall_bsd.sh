mkdir ~/m1 ~/m2
sudo mount -t msdos /dev/da1s1 ~/m1
sudo lklfuse -o allow_other,type=ext4 /dev/da1s2 ~/m2
./install.sh
sudo umount ~/m1 ~/m2
