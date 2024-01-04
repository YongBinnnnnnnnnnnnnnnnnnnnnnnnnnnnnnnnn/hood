#!/usr/bin/python3 -B
"""
HTTP proxy for hood firewall
https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood
Bin Yong all rights reserved.
"""

import asyncio
import ssl
import re
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

def LOG(*args):
  print(*args, flush=True)

parser = argparse.ArgumentParser(
  prog="hood-http-handler",
  description="A HTTP proxy sepecially designed for the hood firewall.",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--address", default="127.0.0.1", type=str, help="the ip address to listent to")
parser.add_argument("--port", default=80, type=int, help="the port number of the proxy")
parser.add_argument("--buffer-size", default=4096, help="buffer size per send/recv")
parser.add_argument("--forward-to-address", default="127.0.0.1", help="the address of the host that HTTPS connections will be forwarded to, use 'off' to use actual ip address of the remote server")

args_ = parser.parse_args()
if args_.forward_to_address == "off":
  args_.forward_to_address = None

ocsp_or_crl_hosts = set([
  "status.geotrust.com",
  "status.rapidssl.com",
  "crl.identrust.com",
  "x1.c.lencr.org",
  "r3.o.lencr.org",
  "ocsp.msocsp.com",
  "ocsp.quovadisglobal.com",
  "ocsp.sca1b.amazontrust.com",
  "zerossl.ocsp.sectigo.com",
  "crt.sectigo.com",
  "ocsp.sectigo.com",
  "ocsp.pki.goog",
  "pki-goog.l.google.com",
  "ocsp.usertrust.com",
  "ocsp.digicert.com",
  "crl3.digicert.com",
  "crl4.digicert.com",
  "ocsp.entrust.net",
  "ocsp.swisssign.net",
  "gold-ev-g2.ocsp.swisssign.net",
  "ocsp.verisign.net",
  "ocsp.globalsign.com",
  "ocsp2.globalsign.com",
  "crl.globalsign.com",
  "ocsp.godaddy.com",
  "certificates.godaddy.com",
  "ocsp.starfieldtech.com",
  "crl.apple.com",
  "ocsp.apple.com",
  "ocsp2.apple.com",
  "ocsp2-lb.apple.com.akadns.net",
  "ocsp2.g.aaplimg.com",
  "ocsp.rootca1.amazontrust.com",
  "status.thawte.com",
  "ocsp.comodoca.com"
])

async def handle_connection(client_reader, client_writer):
  try:
    headers = bytes()
    host = ""
    while True:
      line = await client_reader.readline()
      headers += line
      line = line.decode()
      if line.startswith("host:") or line.startswith("Host:"):
        if host:
          LOG("multple host, discard")
          return

        host = line[5:].strip()
      if line == '\r\n':
        break;
    if not host:
      LOG("host not found, discard")
      return
    #LOG(host)

    port = 80
    ssl_context = None
    server_hostname = None
    do_resolve = True
    host=re.sub(":\d+$", "", host)
    
    if hood_name_service.IsAddressInBlacklist(host)\
      or hood_name_service.IsDomainInBlacklist(host, log=LOG):
      LOG(host, "is in blacklist")
      client_writer.close()
      return

    if host not in ocsp_or_crl_hosts:
      port=443
      ssl_context = ssl.create_default_context()
      ssl_context.minimum_version = ssl.TLSVersion.TLSv1_2
      server_hostname = host
      if args_.forward_to_address:
        host = args_.forward_to_address
        do_resolve = False

    if do_resolve:
      host_addresses = await hood_name_service.HoodResolve(host)
      if not host_addresses:
        LOG("Unable to resolve", host)
        client_writer.close()
        return
      host = host_addresses[0]

    host_reader, host_writer = await asyncio.open_connection(
      host,
      port,
      ssl=ssl_context,
      server_hostname=server_hostname
    )
    LOG(headers)
    host_writer.write(headers)
    await host_writer.drain()
    
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
    LOG(e, e.args)


async def run_server():
  LOG("Starting HTTP server")
  server = await asyncio.start_server(handle_connection, args_.address, args_.port)
  async with server:
    await server.serve_forever()


asyncio.run(run_server())