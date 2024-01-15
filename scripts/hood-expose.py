#!/usr/bin/python3 
"""
TCP Port forwarding tool for hood firewall
basically, a simpler version of socat
https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood
Bin Yong all rights reserved.
"""

import asyncio
import argparse

def load_hood_name_service():
  import importlib.util
  import os
  import sys
  spec = importlib.util.spec_from_file_location("hood-name-service.py",  os.path.dirname(os.path.realpath(__file__)) + "/hood-name-service.py")
  module = importlib.util.module_from_spec(spec)
  sys.modules["hood_name_service"] = module
  spec.loader.exec_module(module)
  return module

hood_name_service = load_hood_name_service()

parser = argparse.ArgumentParser(
  prog="hood-expose",
  description="TCP Port forwarding tool for hood firewall.",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--to-address", required=True, type=str, help="the ip address to expose to")
parser.add_argument("--to-port", type=int, help="the port number to expose to")
parser.add_argument("--from-address", default="127.0.0.1", type=str, help="the address to be exposed")
parser.add_argument("--from-port", required=True, type=int, help="the port to be exposed")
parser.add_argument("--buffer-size", default=4096, help="buffer size per send/recv")

args_ = parser.parse_args()

if not args_.to_port:
  args_.to_port = args_.from_port

async def handle_connection(client_reader, client_writer):
  try:
    host_reader, host_writer = await asyncio.open_connection(
      args_.from_address,
      args_.from_port
    )
    
    buffer_size = args_.buffer_size
    async def forward_client_to_server():
      try:
        while True:
          buffer = await client_reader.read(buffer_size)
          if not buffer:
            break
          host_writer.write(buffer)
          await host_writer.drain()
        host_writer.close()
        client_writer.close()
      except Exception as e:
        pass
    
    async def forward_server_to_client():
      try:
        while True:
          buffer = await host_reader.read(buffer_size)
          if not buffer:
            break
          client_writer.write(buffer)
          await client_writer.drain()
        host_writer.close()
        client_writer.close()
      except Exception as e:
        pass
    await asyncio.wait([
      asyncio.ensure_future(forward_client_to_server()),
      asyncio.ensure_future(forward_server_to_client())
    ])
    await host_writer.wait_closed()
    await client_writer.wait_closed()
  except Exception as e:
    print(e, e.args)


async def run_server():
  args_.from_address = await hood_name_service.HoodResolve(args_.from_address)
  server = await asyncio.start_server(handle_connection, args_.to_address, args_.to_port)
  async with server:
    await server.serve_forever()


asyncio.run(run_server())
