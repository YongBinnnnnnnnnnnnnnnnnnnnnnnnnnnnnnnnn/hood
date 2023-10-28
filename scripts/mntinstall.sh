mkdir ~/m1 ~/m2

machine=$(uname -s)

if [ $machine = "FreeBSD" ]; then
  sudo mount -t msdos /dev/da1s1 ~/m1
  sudo lklfuse -o allow_other,type=ext4 /dev/da1s2 ~/m2
elif [ $machine = "Linux" ]; then
  sudo mount /dev/sdb1 ~/m1
  sudo mount /dev/sdb2 ~/m2
fi


./install.sh "$@"
sudo umount ~/m1 ~/m2
