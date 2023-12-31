#!/usr/bin/python3
"""
HTTP timesync tool for hood firewall
https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood
Bin Yong all rights reserved.
"""

import argparse
import http.client
import subprocess
import sys
import os

parser = argparse.ArgumentParser(
  prog="hood-timesync",
  description="HTTP timesync tool sepecially designed for the hood firewall.",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--host", default="www.bing.com", type=str, help="The website to request time from")
parser.add_argument("--last-resort-host", default="1.1.1.1", type=str, help="The website used as the last resort, should be an IP address")
parser.add_argument("--time-anchor", default=1703703361, type=int, help="An Epoch time that being anchored as past, the time earlier than the anchor will be rejected.\nBased on the assumption that time only moves forward, any time before this value will be rejected.\nSet 0 to disable this test.\nExample: --hard-time-anchor=$(stat /etc/os-release -c %W)")

args_ = parser.parse_args()

headers_ = {
  "Accept":"text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
  "Accept-Encoding":"gzip, deflate, br",
  "Accept-Language":"en-US,en;q=0.9",
  "Sec-Ch-Ua":"\"Not_A Brand\";v=\"8\", \"Chromium\";v=\"120\", \"Google Chrome\";v=\"120\"",
  "Sec-Ch-Ua-Arch":"\"x86\"",
  "Sec-Ch-Ua-Bitness":"\"64\"",
  "Sec-Ch-Ua-Full-Version":"\"120.0.6099.71\"",
  "Sec-Ch-Ua-Full-Version-List":"\"Not_A Brand\";v=\"8.0.0.0\", \"Chromium\";v=\"120.0.6099.71\", \"Google Chrome\";v=\"120.0.6099.71\"",
  "Sec-Ch-Ua-Mobile":"?0",
  "Sec-Ch-Ua-Model":"\"\"",
  "Sec-Ch-Ua-Platform":"\"Windows\"",
  "Sec-Ch-Ua-Platform-Version":"\"10.0.0\"",
  "Sec-Fetch-Dest":"document",
  "Sec-Fetch-Mode":"navigate",
  "Sec-Fetch-Site":"none",
  "Sec-Fetch-User":"?1",
  "Upgrade-Insecure-Requests":"1",
  "User-Agent":"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
}

def time_anchor_test(date):
  if not args_.time_anchor:
    return True
  # use date command to convert date string to seconds since the Epoch
  run_result = subprocess.run(['date', '-d', date, "+%s"], text=True, capture_output=True)
  if run_result.returncode != 0:
    return False
  new_date = int(run_result.stdout)
  return new_date > args_.time_anchor

def set_date(date):
  if not time_anchor_test(date):
    print(date, "failed time anchor test")
    return
  command = ['sudo', 'date', '-s', date]
  if os.getuid() == 0:
    command.pop(0)
  return subprocess.call(command) == 0

def fetch(host, https=True):
  try:
    if https:
      connection = http.client.HTTPSConnection(host)
    else:
      connection = http.client.HTTPConnection(host)
    # Use GET instead of HEAD to look normal
    connection.request("GET", "/", headers=headers_)
    response = connection.getresponse()
    if 'Date' in response.headers:
      return response.headers['Date']
    if 'date' in response.headers:
      return response.headers['date']
  except Exception:
    pass

  return

date = fetch(args_.host, https=True)
if date:
  print(date)
  if set_date(date):
    sys.exit()

print("Fetch time from host failed, use last resort")

date = fetch(args_.last_resort_host, https=False)
if date:
  print(date)
  set_date(date)

date = fetch(args_.host, https=True)
if date:
  print(date)
  set_date(date)
  print("exit")
  sys.exit()

print("All attempts failed!")