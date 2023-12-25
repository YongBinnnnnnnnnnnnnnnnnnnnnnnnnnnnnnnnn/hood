# hood
Things of my MSc thesis about A firewall device

It's not hard to point out someone is wearing a hood, but the hood makes it harder to say who is wearing it.
## Table of Contents
1. [Status](#status)
1. [Graphics to show the concept](#graphics-to-show-the-concept)
2. [Installation](#installation)
3. [License](#license)

## Status

### Firewall hardware
 - [x] Raspberry Pi 4B
 - [ ] Others not tested

### Operating systems to use the firewall
 - [x] Windows 10
 - [x] Linux
 - [x] Mac OS (needs an additional USB NIC)

### Todo
 1. Show realtime network activities on a screen
 2. Support HTTP3 QUIC 


## Graphics to show the concept

![HTTP traffic graph](./paper/graphics/puml/process-http-traffic.png)
![TLS traffic graph](./paper/graphics/puml/process-tls-traffic.png)

## Installation

If you already know how to write raspberrypi os image to a SD card, goto step 3

### Step 1. Download the latest raspberrypi image (64bit)
```shell
curl -L -O -C - https://downloads.raspberrypi.com/raspios_arm64/images/raspios_arm64-2023-12-06/2023-12-05-raspios-bookworm-arm64.img.xz
```
### Step 2. Write the image to a SD card

```shell
xz -c -d 2023-12-05-raspios-bookworm-arm64.img.xz | sudo dd of=/dev/sdX bs=4M status=progress
```
You may need `lsblk` command to locate the device name of your SD card.

### Step 3. Get Hood
You can get it as a zip file from the `Download ZIP` button of the dropdown menu triggered by the `Code ` button on the top.

Or you can use following command

```shell
git clone --depth=1 https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood.git
```

### Step 3. Install hood

Firstly, use `lsblk` command to check the device name of you SD card, 

Then, use following command to ensure the partitions are not mounted (replace sdX with SD card device name)

```shell
sudo umount /dev/sdX*
```

Then, use following command to check the partitions are not mounted, the correct output is nothing (replace sdX with SD card device name)

```shell
mount|grep sdX
```

Then, start install (replace sdX with SD card device name)

```shell
cd hood/scripts
./mntinstall.sh sdX
```

## License

hood by Bin Yong is licensed under [CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/)
