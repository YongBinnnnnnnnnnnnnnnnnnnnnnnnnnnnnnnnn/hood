#!/bin/bash
#wget -qO - https://gitlab.com/paulcarroty/vscodium-deb-rpm-repo/raw/master/pub.gpg | gpg --dearmor | sudo tee /usr/share/keyrings/vscodium-archive-keyring.gpg | sha256sum
#echo 'deb [ signed-by=/usr/share/keyrings/vscodium-archive-keyring.gpg ] https://download.vscodium.com/debs vscodium main' | sudo tee /etc/apt/sources.list.d/vscodium.list
#sudo apt-get update
#sudo apt-get install -y codium
user_agent="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36"

script_dir=$(readlink -f $(dirname "$0"))

cd ~/Downloads
$script_dir/download-vsx.sh xaver/clang-format
$script_dir/download-vsx.sh ms-python/python
$script_dir/download-vsx.sh James-Yu/latex-workshop
$script_dir/download-vsx.sh webfreak/debug

if test -f /usr/share/code/code; then
  exit
fi
rm vscode.deb
if file /usr/bin/ls| grep -q "armhf"; then
  curl -H "User-Agent: ${user_agent}" -L -C - https://update.code.visualstudio.com/latest/linux-deb-armhf/stable -o vscode.deb
else
  curl -H "User-Agent: ${user_agent}" -L -C - https://update.code.visualstudio.com/latest/linux-deb-arm64/stable -o vscode.deb
fi
if curl https://code.visualstudio.com/sha|grep -q $(sha256sum vscode.deb |sed "s/ .*//"); then
  sudo apt install ./vscode.deb 
else
  echo hash mismatch
fi

mkdir -p ~/.config/Code/User
tee ~/.config/Code/User/settings.json <<EOF
{
    "extensions.autoCheckUpdates": false,
    "extensions.autoUpdate": false,
    "update.showReleaseNotes": false,
    "update.mode": "none",
    "telemetry.telemetryLevel": "off"
}
EOF

sudo tee /etc/apparmor.d/vscode-yongb <<EOF
# vim:syntax=apparmor
#include <tunables/global>

/usr/share/code/code {
  #include <abstractions/base>

  network inet dgram,
  owner /** rwm,
  /usr/share/code/** rmix,
  /proc/sys/fs/inotify/** r,
  /proc/ r,
  /sys/devices/system/cpu/** r,
  /etc/fonts/** r,
  /usr/share/** r,
  /usr/local/share/fonts/** r,
  ptrace,
  signal,
  unix,
  capability sys_admin,
  capability sys_chroot,
  @{PROC}/[0-9]*/ r,                 # sandbox wants these
  @{PROC}/[0-9]*/fd/ r,              # sandbox wants these
  @{PROC}/[0-9]*/statm r,            # sandbox wants these
  @{PROC}/[0-9]*/task/[0-9]*/stat r, # sandbox wants these
}

EOF

sudo apparmor_parser -r /etc/apparmor.d/vscode-yongb
