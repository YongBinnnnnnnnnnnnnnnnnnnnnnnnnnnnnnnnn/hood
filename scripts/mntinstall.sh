mkdir -p /tmp/hood-install/mnt

machine=$(uname -s)

if [ $machine = "FreeBSD" ]; then
  sudo lklfuse -o allow_other,type=ext4 /dev/${1}s2 /tmp/hood-install/mnt
  sudo mkdir -p /tmp/hood-install/mnt/boot/firmware
  sudo mount -t msdos /dev/${1}s1 /tmp/hood-install/mnt/boot/firmware
elif [ $machine = "Linux" ]; then
  sudo mount /dev/${1}2 /tmp/hood-install/mnt
  sudo mkdir -p /tmp/hood-install/mnt/boot/firmware
  sudo mount /dev/${1}1 /tmp/hood-install/mnt/boot/firmware
fi


./install.sh "$@:2" prefix=/tmp/hood-install/mnt/
sudo umount /tmp/hood-install/mnt/boot/firmware
sudo umount /tmp/hood-install/mnt
