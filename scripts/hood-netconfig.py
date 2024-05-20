#!/usr/bin/env -S python3 -B
"""
Network configuration tool for hood firewall
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

#from types import SimpleNamespace

parser = argparse.ArgumentParser(
  prog="hood-netconfig",
  description="**EXPERIMENTAL AND UNSTABLE!! DO NOT USE!!!** Network configuration tool specially designed for the hood firewall. ",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--interface", type=str, required=True, help="The interface to be configured")
parser.add_argument("--address", type=str, help="The subnet or ip to be joined use it when you want a random IP address. If the last number is 0, it will be replaced as a random number.")
parser.add_argument("--reserve", default=32, type=int, help="Number of addresses to be reserved for DHCP")
parser.add_argument("--link", type=str, help="up/down")
parser.add_argument("--mac-address", type=str, help="The new mac address to be assigned to the interface. If the value is 'random', a random address will be assigned to the interface.")
parser.add_argument("--gateway", default="", type=str, help="The gateway. Use value 'auto' to guess it from other optoins.")
parser.add_argument("--gateway-mac-address", default="", type=str, help="The mac address of the gateway")

def execute_as_root(command):
  if os.getuid() != 0:
    command.insert(0, "sudo")
  #print(command)
  subprocess.call(command)
  
byte_masks = (0b00000000,0b10000000,0b11000000,0b11100000,0b11110000,0b11111000,0b11111100,0b11111110,0b11111111)

def link_set_state(interface, new_state):
  if 'freebsd' in sys.platform:
    execute_as_root(["ifconfig", interface, "link", "set", new_state])
  else:
    execute_as_root(["ip", "link", "set", new_state, "dev", interface])

def flush_ip_address(interface):
  if 'freebsd' in sys.platform:
    execute_as_root(["ifconfig", interface, "delete"])
  else:
    execute_as_root(["ip", "addr", "flush", "dev", interface])

def hood_netconfig(args_): 
  if args_.link:
    link_set_state(args_.interface, args_.link)
    
  if args_.mac_address:
    if args_.mac_address == "random":
      mac_address = secrets.token_bytes(6)
      mac_address = ':'.join(map(lambda x:f'{x:02x}', mac_address))
    else:
      mac_address = args_.mac_address
    if 'freebsd' in sys.platform:
      execute_as_root(["ifconfig", args_.interface, "link",  mac_address])
    else:
      with open('/sys/class/net/' + args_.interface + '/operstate', 'r' ) as operstate_file:
        operstate = operstate_file.read()
      if operstate == 'up\n':
        link_set_state(args_.interface, 'down')
      execute_as_root(["ip", "link", "set", "address",  mac_address, "dev", args_.interface])
      if operstate == 'up\n':
        link_set_state(args_.interface, 'up')
    
    
  
  if args_.address:
    if args_.reserve < 2:
      args_.reserve = 2
    gateway = args_.gateway
  
    [address, mask_length] = args_.address.split("/")
    byte_address = socket.inet_pton(socket.AF_INET, address)
    mask_length = int(mask_length)
    assert mask_length < 32
    if byte_address[3] == 0:
      byte_address = byte_address[:3] + bytes((args_.reserve + 1 + secrets.randbelow(254 - args_.reserve),))
    address = socket.inet_ntoa(byte_address)
    print("address:", address)
    flush_ip_address(args_.interface)
    if 'freebsd' in sys.platform:
      #TODO: code copied from name service, consider crate a common library instead of this stupid lazy way.
      execute_as_root(["ifconfig", args_.interface, "add",  address + '/' + str(mask_length)])
    else:
      execute_as_root(["ip", "addr", "add",  address + '/' + str(mask_length), "dev", args_.interface])
      
  
  if args_.gateway:
    if args_.gateway == 'auto':
      #TODO auto from current ip
      gateway=byte_address[:int(mask_length/8)]
      mask_end = mask_length % 8
      if mask_end:
        gatepway = gateway + bytes((byte_address[len(gateway)] & byte_masks[mask_end],))
      if len(gateway) < 4:
        gateway = gateway + b'\x00' * (4 - len(gateway))
      args_.gateway = gateway[:3] + bytes((gateway[3] | 1,))
      
      #'.'.join(map(str, gateway) is much slower
      gateway = socket.inet_ntoa(gateway)
      print("gateway:", gateway)
    # TODO: non default
    if 'freebsd' in sys.platform:
      execute_as_root(["route", "add", "default", args_.gateway, "-ifp", args_.interface])
      if args_.gateway_mac_address:
        raise NotImplementedError
        #TODO
    else:
      command = 
      if args_.gateway_mac_address:
        execute_as_root(["ip", "neigh", "add", gateway, "dev", args_.interface, "lladdr", args_.gateway_mac_address])
      execute_as_root(["ip", "route", "add", "default", "via", gateway, "dev", args_.interface])
      
hood_netconfig(parser.parse_args())
