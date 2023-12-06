#!/usr/bin/python3
import asyncio
import sys


class DnsClientProtocol:
    def __init__(self, message, on_con_lost):
        self.message = message
        self.on_con_lost = on_con_lost
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport
        print('Send:', self.message)
        self.transport.sendto(self.message.encode())

    def datagram_received(self, data, addr):
        print("Received:", data.decode().split("\0"))

        print("Close the socket")
        self.transport.close()

    def error_received(self, exc):
        print('Error received:', exc)

    def connection_lost(self, exc):
        print("Connection closed")
        self.on_con_lost.set_result(True)


async def main():
    # Get a reference to the event loop as we plan to use
    # low-level APIs.
    if len(sys.argv) < 2:
      return
    loop = asyncio.get_running_loop()

    on_con_lost = loop.create_future()
    message = sys.argv[1]

    transport, protocol = await loop.create_datagram_endpoint(
        lambda: DnsClientProtocol(message, on_con_lost),
        remote_addr=('127.0.0.1', 530))

    try:
        await on_con_lost
    finally:
        transport.close()


asyncio.run(main())