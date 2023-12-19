#!/usr/bin/python3
"""
Behavioral simuluation tool for hood firewall
https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood
Bin Yong all rights reserved.
"""

import argparse
from http.cookiejar import CookieJar
import subprocess
import sys
import threading
from html.parser import HTMLParser
import urllib
import socket

parser = argparse.ArgumentParser(
  prog="hood-timesync",
  description="Behavioral simuluation tool for hood firewall.",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--do", default="random-persist", type=str, help="Which role to act.")
args_ = parser.parse_args()

#TODO: HAR or pcap replay

default_accept_ = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7"

headers_ = {
  "Accept":default_accept_,
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

class Executor:
  pool = []
  update_countdown_lock = threading.Lock()
  new_task_event = threading.Event()
  sleep_duration = 1
  task_queue = []

  queue_lock = threading.Lock()
  def __init__(self, pool_size=8):
    for _ in range(pool_size):
      self.pool.append(threading.Thread(target=self.worker).start())

  def queue_one(task):
    with self.queue_lock:
      task_queue.append(task)
    new_task_event.set()
        
  def get():
    if task_queue:
        with self.queue_lock:
          if task_queue:
            return task_queue.pop(0)
        
  def worker(self):
    while True:
      self.new_task_event.wait()
      next_task = None
      while True:
        task = None
        if next_task:
          task = next_task
          next_task = None
        else:
          with self.queue_lock:
            task = self.get()

        result = task["run"]()

        if "callback" in task and task["callback"]:
          task["callback"](result)

        if "dependants" in task and task["dependants"]:
          with self.update_countdown_lock:
            for dependant in task["dependants"]:
              dependant.wating -= 1
              if not dependant.wating:
                if next_task:
                  self.queue_one(dependant)
                else:
                  next_task = dependant

    
  def start(self):
    pass
    
executor_ = Executor()

class HoodHTMLParser(HTMLParser):
  useless_tags = set(['div', 'i', 'span', 'p', 'li', 'ul', 'input', 'html', 'head', 'body', 'textarea'])
  useful_tags = set(['script', 'a', 'img', 'link', 'meta', 'video'])

  def handle_starttag(self, tag, attrs):
    if tag in self.useful_tags:
      print(tag, attrs)
      return
    if self.useless_tags:
      if tag not in self.useless_tags:
        print("unhandled", tag , attrs)
  #def handle_endtag(self, tag):
  #  print(tag)
  #def handle_data(self, data):
  #  pass


class Browser(object):
  #todo: support persist connections
  def __init__(self):
    super(Browser, self).__init__()

  def __init__(self):
    self.cookiejar = CookieJar()
    cookie_processor = urllib.request.HTTPCookieProcessor(cookiejar=CookieJar())
    self.opener = urllib.request.build_opener(cookie_processor)
    print(self.opener)

  def load(self, url):
    try:
      return self.opener.open(url)
    except Exception as e:
      pass

  def loadWebpage(self, url):
    with self.load(url) as f:
      content = f.read().decode('utf-8')
      html_parser = HoodHTMLParser()
      print(content)
      html_parser.feed(content)
  
  

browser = Browser()

print(urllib.request.urlopen("https://www.baidu.com"))

browser.loadWebpage("http://www.baidu.com")
