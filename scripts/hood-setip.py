#!/usr/bin/env -S python3 -B
"""
IP configuration tool for hood firewall
https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood
Bin Yong all rights reserved.
"""

import argparse
import os
import secrets
import socket
import struct
import subprocess
import sys

parser = argparse.ArgumentParser(
  prog="hood-setip",
  description="**EXPERIMENTAL!! DO NOT USE!!!** IP configuration tool specially designed for the hood firewall. ",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--interface", type=str, required=True, help="The interface to be configured")
parser.add_argument("--address", type=str, required=True, help="The subnet or ip to be joined use it when you want a random IP address. If the last number is 0, it will be replaced as a random number.")
parser.add_argument("--reserve", default=32, type=int, help="Number of addresses to be reserved for DHCP")
parser.add_argument("--gateway", default="", type=str, help="The gateway. Use value 'auto' to guess it from other optoins.")

def execute_as_root(command):
  if os.getuid() != 0:
    command.insert(0, "sudo")
  #print(command)
  subprocess.call(command)
  

args_ = parser.parse_args()

if args_.reserve < 2:
  args_.reserve = 2
gateway = args_.gateway

[address, mask_length] = args_.address.split("/")
byte_address = socket.inet_pton(socket.AF_INET, address)
mask = int(mask_length)
assert mask_length <= 32
if byte_address[3] == 0:
  byte_address = byte_address[:3] + bytes([args_.reserve + 1 + secrets.randbelow(254 - args_.reserve)])
address = socket.inet_ntoa(byte_address)
print("address:", address)
if 'freebsd' in sys.platform:
  #TODO: code copied from name service, consider crate a common library instead of this stupid lazy way.
  execute_as_root(["ifconfig", args_.interface, "add",  address + '/' + str(mask_length)])
else:
  execute_as_root(["ip", "addr", "add",  address + '/' + str(mask_length), "dev", args_.interface])
  
   
if args_.gateway == 'auto':
  gateway = byte_address[:3] + b'\x01'
  #'.'.join(map(str, gateway) is much slower
  gateway = socket.inet_ntoa(gateway)
  print("gateway:", gateway)


if gateway:
  # TODO: non default
  if 'freebsd' in sys.platform:
    execute_as_root(["route", "add", "default", gateway, "-ifp", args_.interface])
  else:
    execute_as_root(["ip", "route", "add", "default", "via", gateway])
