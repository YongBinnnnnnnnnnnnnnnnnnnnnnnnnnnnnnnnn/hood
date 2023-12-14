#!/bin/bash

modprobe dwc2
modprobe libcomposite

if [ "$1" = "ethernet" ]; then
  gadget=/sys/kernel/config/usb_gadget/ethernet
  mkdir -p ${gadget}
  #echo "0x1d6b" > ${gadget}/idVendor #linux
  #echo "0x0104" > ${gadget}/idProduct # mulstifunction gomposite
  #echo "0x1d50" > ${gadget}/idVendor
  #echo "0x60c7" > ${gadget}/idProduct
  #echo "0x1d6b" > ${gadget}/idVendor #linux
  #echo "0x0137" > ${gadget}/idProduct # mulstifunction gomposite
  echo "0x0694" > ${gadget}/idVendor #LEGO
  echo "0x0005" > ${gadget}/idProduct # EV3
  echo "0x1000" > ${gadget}/bcdDevice
  echo "0x0200" > ${gadget}/bcdUSB
  echo "0xEF" > ${gadget}/bDeviceClass
  echo "0x02" > ${gadget}/bDeviceSubClass
  echo "0x01" > ${gadget}/bDeviceProtocol
  mkdir -p ${gadget}/strings/0x409
  echo "" > ${gadget}/strings/0x409/manufacturer
  echo "" > ${gadget}/strings/0x409/product
  echo "fedcba9876543211" > ${gadget}/strings/0x409/serialnumber
  mkdir -p ${gadget}/configs/c.1
  echo "250" > ${gadget}/configs/c.1/MaxPower
  #echo "0x80" > ${gadget}/configs/c.1/bmAttributes
  echo "0xC0" > ${gadget}/configs/c.1/bmAttributes
  mkdir -p ${gadget}/configs/c.1/strings/0x409
  echo "RNDIS" > ${gadget}/configs/c.1/strings/0x409/configuration
  mkdir -p ${gadget}/configs/c.2
  echo "250" > ${gadget}/configs/c.2/MaxPower
  mkdir -p ${gadget}/configs/c.2/strings/0x409
  echo "ECM" > ${gadget}/configs/c.2/strings/0x409/configuration

  mkdir -p ${gadget}/functions/ecm.usb0
  echo "00:dd:dc:eb:6d:a2" > ${gadget}/functions/ecm.usb0/dev_addr
  echo "00:dc:c8:f7:75:15" > ${gadget}/functions/ecm.usb0/host_addr
  mkdir -p ${gadget}/functions/rndis.usb0
  echo "00:dd:dc:eb:6d:a1" > ${gadget}/functions/rndis.usb0/dev_addr
  echo "00:dc:c8:f7:75:14" > ${gadget}/functions/rndis.usb0/host_addr
  mkdir -p ${gadget}/functions/rndis.usb0/os_desc
  echo "RNDIS" > ${gadget}/functions/rndis.usb0/os_desc/interface.rndis/compatible_id
  echo "5162001" > ${gadget}/functions/rndis.usb0/os_desc/interface.rndis/sub_compatible_id

  mkdir -p ${gadget}/os_desc
  echo "1" > ${gadget}/os_desc/use
  echo "0xcd" > ${gadget}/os_desc/b_vendor_code
  echo "MSFT100" > ${gadget}/os_desc/qw_sign
  ln -s ${gadget}/configs/c.1 ${gadget}/os_desc
  ln -s ${gadget}/functions/rndis.usb0 ${gadget}/configs/c.1
  ln -s ${gadget}/functions/ecm.usb0 ${gadget}/configs/c.2

  ls /sys/class/udc > ${gadget}/UDC
  udevadm settle -t 5 || :
else
  echo "options: ethernet"
fi