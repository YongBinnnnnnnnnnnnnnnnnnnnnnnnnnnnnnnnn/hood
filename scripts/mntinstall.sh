mkdir -p /tmp/hood-install/mnt

machine=$(uname -s)

if [ $machine = "FreeBSD" ]; then
  sudo lklfuse -o allow_other,type=ext4 /dev/da1s2 /tmp/hood-install/mnt
  sudo mkdir -p /tmp/hood-install/mnt/boot/firmware
  sudo mount -t msdos /dev/da1s1 /tmp/hood-install/mnt/boot/firmware
elif [ $machine = "Linux" ]; then
  sudo mount /dev/sdb2 /tmp/hood-install/mnt
  sudo mkdir -p /tmp/hood-install/mnt/boot/firmware
  sudo mount /dev/sdb1 /tmp/hood-install/mnt/boot/firmware
fi


./install.sh "$@" prefix=/tmp/hood-install/mnt/
sudo umount /tmp/hood-install/mnt/boot/firmware
sudo umount /tmp/hood-install/mnt
