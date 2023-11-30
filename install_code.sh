#wget -qO - https://gitlab.com/paulcarroty/vscodium-deb-rpm-repo/raw/master/pub.gpg | gpg --dearmor | sudo tee /usr/share/keyrings/vscodium-archive-keyring.gpg | sha256sum
#echo 'deb [ signed-by=/usr/share/keyrings/vscodium-archive-keyring.gpg ] https://download.vscodium.com/debs vscodium main' | sudo tee /etc/apt/sources.list.d/vscodium.list
#sudo apt-get update
#sudo apt-get install -y codium
user_agent="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36"

cd download
rm vscode.deb
if uname  -a| grep "aarch64"; then
  curl -H "User-Agent: ${user_agent}" -L -C - https://update.code.visualstudio.com/latest/linux-deb-arm64/stable -o vscode.deb
else
  curl -H "User-Agent: ${user_agent}" -L -C - https://update.code.visualstudio.com/latest/linux-deb-armhf/stable -o vscode.deb
fi
if curl https://code.visualstudio.com/sha|grep -q $(sha256sum vscode.deb |sed "s/ .*//"); then
  sudo apt install ./vscode.deb 
else
  echo hash mismatch
fi

tee ~/.config/Code/User/settings.json <<EOF
{
    "extensions.autoCheckUpdates": false,
    "extensions.autoUpdate": false,
    "update.showReleaseNotes": false,
    "update.mode": "none",
    "telemetry.telemetryLevel": "off"
}
EOF