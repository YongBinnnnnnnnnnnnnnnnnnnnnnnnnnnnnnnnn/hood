mkdir ~/hood_install

machine=$(uname -s)

if [ $machine = "FreeBSD" ]; then
  sudo lklfuse -o allow_other,type=ext4 /dev/da1s2 ~/hood_install
  sudo mkdir -p ~/hood_install/boot/firmware
  sudo mount -t msdos /dev/da1s1 ~/hood_install/boot/firmware
elif [ $machine = "Linux" ]; then
  sudo mount /dev/sdb2 ~/hood_install
  sudo mkdir -p ~/hood_install/boot/firmware
  sudo mount /dev/sdb1 ~/hood_install/boot/firmware
fi


./install.sh "$@" prefix=~/hood_install/
sudo umount ~/hood_install/boot/firmware
sudo umount ~/hood_install
