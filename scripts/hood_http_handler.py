#!/usr/bin/python3
import asyncio
import ssl

buffer_size_ = 4096
forward_to_hood_proxy_ = true

ocsp_or_crl_hosts = Set([
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
          print("multple host, discard")
          return

        host = line[5:].strip()
      if line == '\r\n':
        break;
    if not host:
      print("host not found, discard")
      return
    #print(host)

    port = 80
    ssl_context = None
    if host not in ocsp_or_crl_hosts:
      ssl_context = ssl.create_default_context()
      ssl_context.minimum_version = ssl.TLSVersion.TLSv1_2
      port = 443
    host_reader, host_writer = await asyncio.open_connection(host, port, ssl=ssl_context)
    print(headers)
    host_writer.write(headers)
    await host_writer.drain()
    
    async def forward_client_to_server():
      try:
        while True:
          buffer = await client_reader.read(buffer_size_)
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
          buffer = await host_reader.read(buffer_size_)
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
    print(e.message, e.args)


async def run_server():
  server = await asyncio.start_server(handle_connection, "127.0.0.1", 80)
  async with server:
    await server.serve_forever()

asyncio.run(run_server())