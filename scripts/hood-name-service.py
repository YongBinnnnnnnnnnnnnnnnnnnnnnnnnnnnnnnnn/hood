#!/usr/bin/python3
"""
Name servce for hood firewall
https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood
Bin Yong all rights reserved.
"""

import sys
import asyncio
import socket
import struct
import re

with open("/var/lib/hood/ip_subnet_blacklist.txt") as f:
  ip_subnet_blacklist = f.read().splitlines()

def __filter_blacklist(line):
  return line and line[0] != "#"

def __parse_subnet_blacklist(line):
  [address, length] = line.split("/")
  address = socket.inet_pton(socket.AF_INET, address)
  length = int(length)
  assert length <= 32
  address = struct.unpack(">I", address)[0]
  cursor_bit = 1 << 31
  mask = 0
  while length:
    mask = mask | cursor_bit
    length = length - 1
    cursor_bit = cursor_bit >> 1
  return (address & mask, mask)

ip_subnet_blacklist = filter(__filter_blacklist, ip_subnet_blacklist)
ip_subnet_blacklist = tuple(map(__parse_subnet_blacklist, ip_subnet_blacklist))

def IsAddressInBlacklist(address):
  try:
    address = socket.inet_pton(socket.AF_INET, address)
    address = struct.unpack(">I", address)[0]
    for subnet in ip_subnet_blacklist:
      if subnet[0] == (address & subnet[1]):
        return True
  except Exception:
    pass
  return False

with open("/var/lib/hood/domain_blacklist.txt") as f:
  domain_blacklist = f.read().splitlines()

domain_blacklist = filter(__filter_blacklist, domain_blacklist)
domain_blacklist = tuple(map(re.compile, domain_blacklist))

def IsDomainInBlacklist(domain, log=print):
  for line in domain_blacklist:
    if line.fullmatch(domain):
      log(domain, "is blocked by blacklist entry", line)
      return True
  return False

if __name__ == '__main__':
  import ssl
  import urllib.parse
  import os
  import json
  import time
  import argparse

  def LOG(*args):
    print(*args, flush=True)

  parser = argparse.ArgumentParser(
    prog="hood-name-service",
    description="A name service sepecially designed for the hood firewall.",
    epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
  )

  parser.add_argument("--address", default="127.0.0.1", type=str, help="the ip address to listent to")
  parser.add_argument("--port", default=530, type=int, help="the port number to be used")
  #parser.add_argument("--useragent", default="Mozilla/5.0 (X11; Fedora; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/120.0", help="the useragent to be used")

  args_ = parser.parse_args()

  dns_over_https_server_paths = {
  #  "doh.opendns.com",
    "dns.google": "/resolve",
  #  "dns10.quad9.net",
    "1.1.1.1": "/dns-query",
    "mozilla.cloudflare-dns.com": "/dns-query",
    "chrome.cloudflare-dns.com": "/dns-query",
  #  "doh.opendns.com",
  };

  class DnsOverHttpsClient:
    __reader = None
    __writer = None
    def __init__(self, host_name):
      self.host_name = host_name
      self.chunked_transfer = False
      self.default_headers = ("Host: "  + self.host_name + "\r\n").encode("utf-8")
      self.default_headers += b"Accept: application/dns-json\r\n"\
                              b"Connection: keep-alive\r\n"\
                              b"\r\n"
      self.busy = False
    
    async def write(self, data):
      await self.ensure_connection()
      self.__writer.write(data)
      await self.__writer.drain()

    async def read(self, size):
      return await self.__reader.read(size)

    async def readexactly(self, size):
      return await self.__reader.readexactly(size)

    async def readline(self):
      return await self.__reader.readline()
      
    async def ensure_connection(self):
      if not self.__reader \
        or not self.__writer \
        or self.__reader.at_eof() \
        or self.__writer.is_closing():
        return await self.reconnect()

    async def reconnect(self):
      ssl_context = ssl.create_default_context()
      ssl_context.minimum_version = ssl.TLSVersion.TLSv1_2
      self.__reader, self.__writer = await asyncio.open_connection(self.host_name, 443, ssl=ssl_context)
      self.chunked_transfer = False

    async def resolve(self, name):
      name = urllib.parse.unquote_plus(name)
      headers = ("GET " + dns_over_https_server_paths[self.host_name] + "?type=A&name="  + name + " HTTP/1.1 \r\n").encode("utf-8")
      headers += self.default_headers
      #LOG(headers)
      while self.busy:
        await asyncio.sleep(0)
      self.busy = True
      await self.write(headers)
      headers = bytes()
      success = True
      while True:
        line = await self.readline()
        headers += line
        line = line.decode()
        if line.startswith("HTTP/"):
          success = success and " 200 OK" in line
        if line.startswith("Content-Type:"):
          if "application/dns-json" not in line\
            and "application/json" not in line:
            success = False
        if line.startswith("Transfer-Encoding:"):
          self.chunked_transfer = "chunked" in line
        if line.startswith("Content-Length:"):
          answer_size = int(line[16:-2], 10)
        if line == '\r\n':
          break;
      #LOG(headers)
      if not success:
        LOG("Failure:", headers)
        return 
      if self.chunked_transfer:
        answer_size = await self.readline() 
        answer_size = int(answer_size[:-2], 16)
        LOG(answer_size)
      if answer_size > 65536 or answer_size < 2:
        LOG(answer_size)
        return
      answer = (await self.readexactly(answer_size)).decode("utf-8")
      answer = json.loads(answer)
      if self.chunked_transfer:
        await self.readline()
        await self.readline()
        await self.readline()

      self.busy = False
      return answer
      


  class DnsOverHttpsResolver:
    def __init__(self, servers = dns_over_https_server_paths.keys()):
      self.servers = servers
      self.pointer = 0
      self.clients = tuple(map(DnsOverHttpsClient, self.servers))
      self.cache = {}

    async def resolve(self, name_stack):
      try:
        name = name_stack[len(name_stack) - 1]
        if IsDomainInBlacklist(name, log=LOG):
          return
        if name in self.cache:
          cache = self.cache[name]
          if time.monotonic_ns() < cache["expire_at"]:
            LOG(cache["expire_at"], time.monotonic_ns())
            return cache["result"]
          else:
            self.cache.pop(name, None)
        while True:
          client = self.clients[self.pointer]
          self.pointer += 1
          if self.pointer == len(self.clients):
            self.pointer = 0
          if not client.busy:
            break
          asyncio.sleep(0)
        answer = await client.resolve(name)

        if "Status" not in answer:
          return
        if answer["Status"] != 0:
          return
        if "Answer" not in answer:
          return
        answer = answer["Answer"]
        result = []
        min_ttl = 3600
        cname = None
        for record in answer:
          if "data" not in record:
            continue
          if "TTL" not in record:
            continue
          address = record["data"]
          if IsAddressInBlacklist(address):
            continue
          try:
            cname = address
            socket.inet_pton(socket.AF_INET, address)
            result.append(address)
            min_ttl = min(min_ttl, record["TTL"])
            #LOG(record)
          except Exception:
            pass
        if not result and cname:
          if cname not in name_stack:
            name_stack.append(cname)
            return self.resolve(name_stack)
          return result
        if min_ttl > 30:
          LOG(min_ttl)
          if len(self.cache) > 512:
            now = time.monotonic_ns()
            for k, v in self.cache.items():
              if now > v.expire_at:
                break
            if now > v.expire_at:
              del self.cache[k]

          if len(self.cache) < 512:
            self.cache[name] = {
              "expire_at": time.monotonic_ns() + min_ttl * 1000000000,
              "result": result
            }
        return result
      except Exception as e:
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        LOG(e, fname, exc_tb.tb_lineno)

        

  doh_resolver_ = DnsOverHttpsResolver()

  class NameServiceProtocol:
    def connection_made(self, transport):
      self.transport = transport

    def datagram_received(self, data, addr):
      self.addr = addr
      LOG('%s -> %r' % (addr, data))
      def reply_none():
        future = asyncio.Future()
        future.set_result(None)
        self.name_resolved(future)
      if len(data) > 253:
        return reply_none()
      if b"." not in data:
        return reply_none()
      asyncio.ensure_future(doh_resolver_.resolve([data.decode()])).add_done_callback(
        self.name_resolved
      )

    def name_resolved(self, future):
      result = future.result()
      LOG('%s <- %r' % (self.addr, result))
      if not result:
        result = "\0".encode()
      else:
        result = "\0".join(result).encode()
      self.transport.sendto(result, self.addr)

  async def run_server():
    LOG("Starting UDP server")

    # Get a reference to the event loop as we plan to use
    # low-level APIs.
    loop = asyncio.get_running_loop()

    # One protocol instance will be created to serve all
    # client requests.
    transport, protocol = await loop.create_datagram_endpoint(
      lambda: NameServiceProtocol(),
      local_addr=(args_.address, args_.port))

    await asyncio.Future() 
    transport.close()

  asyncio.run(run_server())
else:

  class HoodResolve(asyncio.Future):
    attempts_left = 3
    loop = None
    timeout = 5
    timeout_future = None
    def __init__(self, query, loop = None):
      if not loop:
        loop = asyncio.get_running_loop()
      self.loop = loop
      super().__init__(loop = loop)
      self.transport = None
      self.query = query
      try:
        socket.inet_pton(socket.AF_INET, query)
        self.set_result([query])
        return
      except OSError:
          pass
      self.attempt()
    
    def attempt(self):
      if self.done():
        return
      self.attempts_left = self.attempts_left - 1
      if self.attempts_left == 0:
        self.set_result(None)
        return
      asyncio.ensure_future(
        self.loop.create_datagram_endpoint(
          lambda: self, remote_addr=('127.0.0.1', 530)
        ),
        loop = self.loop
      ).add_done_callback(lambda x:x)
      self.timeout_future = asyncio.ensure_future(
        asyncio.sleep(self.timeout),
        loop = self.loop
      )
      self.timeout_future.add_done_callback(lambda x: self.attempt())
    def connection_made(self, transport):
      self.transport = transport
      self.transport.sendto(self.query.encode())

    def datagram_received(self, data, addr):
      self.timeout_future.cancel()
      if len(data) <= 7:
        result = []
      else:
        result = data.decode().split("\0")
      self.set_result(result)
      self.transport.close()

    def error_received(self, exc):
      LOG('Error received:', exc)
      self.set_result(None)

    def connection_lost(self, exc):
      self.transport = None
      if not self.done():
        self.set_result(None)

  socket_getaddrinfo = socket.getaddrinfo
  
  def HoodGetAddrInfo(host, port, family=0, type=0, proto=0, flags=0):
    if family == socket.AF_INET6:
      return []
    if family == socket.AF_INET or family == 0 or family == socket.AF_UNSPEC:
      loop = asyncio.new_event_loop()
      future = HoodResolve(host, loop=loop)
      loop.run_until_complete(future)
      addresses = future.result()
      if proto == 0:
        proto = 6
      if type == 0:
        type = socket.SOCK_STREAM
      def convert(address):
        return (socket.AF_INET, type, proto, '', (address, port))
      return list(map(convert, addresses))
    return socket_getaddrinfo(host, port, family, type, proto, flags)
    

  def HookGetAddrInfo():
    socket.getaddrinfo = HoodGetAddrInfo
