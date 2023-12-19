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
import time
import json
import os
import random
import secrets

parser = argparse.ArgumentParser(
  prog="hood-timesync",
  description="Behavioral simuluation tool for hood firewall.",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--action", default="browse_bing_news", type=str, help="Action to act.")
parser.add_argument("--age", default=13, type=int, help="Age to act.")
parser.add_argument("--ignore-proper-age", default=False, type=bool, help="Follow the proper age range.")
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
  task_queue = []
  delayed_queue = []
  already_delayed_tasks = []
  queue_lock = threading.Lock()
  thread_local = threading.local()
  delay_precision = 0.1

  def __init__(self, pool_size=8):
    for _ in range(pool_size):
      self.pool.append(threading.Thread(target=self.worker).start())
    threading.Thread(target=self.delay_scheduler).start()

  def queue_one(self, task):
    with self.queue_lock:
      self.task_queue.append(task)
    self.new_task_event.set()
        
  def queue_multiple(self, task_list):
    task_list = list(filter(lambda task:not task.get("waiting", 0), task_list))
    with self.queue_lock:
      self.task_queue = self.task_queue + task_list
      alert_times = min(len(self.pool), len(self.task_queue))
    for i in range(alert_times):
      self.new_task_event.set()
        
  def get(self):
    if self.task_queue:
        with self.queue_lock:
          if self.task_queue:
            return self.task_queue.pop(0)
  
  def run_one(self):
    #if hasattr(self.thread_local, 'worker'):
    self.__run(self.get())

  def __run(self, task):
    if not task:
      return
    task = self.__delay_task_if_not_has_been(task)
    if not task:
      return
    if "run" in task and task["run"]:
      task["run"]()
    if "dependants" in task and task["dependants"]:
      new_tasks = []
      with self.update_countdown_lock:
        for dependant in task["dependants"]:
          dependant["waiting"] = dependant["waiting"] - 1
          if dependant["waiting"] == 0:
            new_tasks.append(dependant)
      self.queue_multiple(new_tasks)

  def __delay_task_if_not_has_been(self, task):
    if "delay" not in task:
      return task
    delay = task["delay"]
    if not delay:
      return task
    if hasattr(delay, '__getitem__'):
      delay = random.uniform(delay[0], delay[1])

    with self.queue_lock:
      if task in self.already_delayed_tasks:
        self.already_delayed_tasks.remove(task)
        return task
      
      self.delayed_queue.append({
        "end": time.monotonic() + delay,
        "task": task
      })

  def delay_scheduler(self):
    while True:
      time.sleep(self.delay_precision)
      if not self.delayed_queue:
        continue
      now = time.monotonic()
      finished_tasks = []
      with self.queue_lock:
        to_remove = []
        for item in self.delayed_queue:
          if now > item["end"]:
            to_remove.append(item)
            self.already_delayed_tasks.append(item["task"])
            finished_tasks.append(item["task"])
        for item in to_remove:
          self.delayed_queue.remove(item)
      if finished_tasks:
        self.queue_multiple(finished_tasks)
      
  def worker(self):
    self.thread_local.worker = True
    while True:
      self.new_task_event.wait()
      while self.task_queue:
        self.__run(self.get())

  def sleep(self, duration):
    #not implemented
    begin = time.monotonic()

    while True:
      new_task = None
      if self.task_queue:
        if self.queue_lock.acquire(blocking=True, timeout=0.1):
          if self.task_queue:
            return self.task_queue.pop(0)

       

    
  def start(self):
    pass
    
executor_ = Executor()

class HoodHTMLParser(HTMLParser):
  #useless_tags = set(['meta', 'div', 'i', 'span', 'p', 'style', 'map', 'area', 'b', 'form', 'title', 'noscript', 'li', 'ul', 'input', 'html', 'head', 'body', 'textarea'])
  useless_tags = None
  tags_with_src = set(['script', 'img', 'audio', 'video', 'iframe'])
  tags_with_href = set(['a', 'link'])
  load_rels = set(['icon', 'stylesheet', 'preload'])

  resources = []
  links = []

  def __init__(self, protocol, netloc):
    super(HoodHTMLParser, self).__init__()
    self.protocol = protocol
    self.netloc = netloc
  def build_url(self, url):
    if url.startswith('http://'):
      return url
    if url.startswith('https://'):
      return url
    if url.startswith('//'):
      return self.protocol + ':' + url
    if url.startswith('/'):
      return self.protocol + '://' + self.netloc + url
    
  def handle_starttag(self, tag, attrs):
    def save_url_attr(target_attr):
      for attr in attrs:
        if attr[0] == target_attr:
          url = self.build_url(attr[1])
          if not url:
            return
          if tag == 'a':
            self.links.append((tag, url))
          else:
            self.resources.append((tag, url))
    if tag in self.tags_with_src:
      save_url_attr('src')
    elif tag in self.tags_with_href:
      if tag == 'link':
        for attr in attrs:
          if attr[0] == 'rel':
            if attr[1] not in self.load_rels:
              return
            tag = tag + ':' + attr[1]
            break
      save_url_attr('href')
    
    if self.useless_tags:
      if tag not in self.useless_tags:
        print("unhandled", tag , attrs)
  def handle_endtag(self, tag):
    pass
  def handle_data(self, data):
    pass

  
class Browser(object):
  #todo: support persist connections
  links = []
  lock = threading.Lock()
  loading_resources = []
  loaded_resources = set()
  url = None
  def __init__(self):
    super(Browser, self).__init__()

  def __init__(self):
    self.cookiejar = CookieJar()
    cookie_processor = urllib.request.HTTPCookieProcessor(cookiejar=CookieJar())
    self.opener = urllib.request.build_opener(cookie_processor)
    print(self.opener)

  def load(self, url, referer = None):
    try:
      headers = headers_.copy()
      headers["Connection"] : "closee";
      if referer:
        headers = headers.copy()
        headers["Referer"] = referer
      request = urllib.request.Request(url, headers=headers);
      return self.opener.open(url)
    except Exception as e:
      pass

  def load_webpage(self, url, iframe = False, callback = None):
    referer = None
    if not iframe:
      print("loading", url)
      with self.lock:
        self.loading_resources = []
        self.links = []
        referer = self.url
        self.url = url
    parsed_url = urllib.parse.urlparse(url)
    if iframe:
      referer = self.url
    with self.load(url, referer) as f:
      content = f.read().decode('utf-8')
    html_parser = HoodHTMLParser(parsed_url.scheme, parsed_url.netloc)
    html_parser.feed(content)
    with self.lock:
      self.loading_resources = self.loading_resources + html_parser.resources
      self.links = self.links + list(map(lambda x: x[1], html_parser.links))
    self.load_resources(url, callback)

  def load_resources(self, referer, callback = None):
    if not self.loading_resources:
      if callback:
        callback()
      return
    with self.lock:
      loading_resources = self.loading_resources
      self.loading_resources = []
    countdown = len(loading_resources)
    countdown_lock = threading.Lock()
    def load_finish_callback():
      run_callback = False
      nonlocal countdown
      nonlocal countdown_lock
      with countdown_lock:
        countdown = countdown - 1
        if countdown == 0:
          run_callback = callback
      if run_callback:
        callback()
    def load_resource(resource):
      url = resource[1]
      if url in self.loaded_resources:
        load_finish_callback()
        return
      if resource[0] == 'iframe':
        self.load_webpage(url, iframe = True, callback = load_finish_callback)
      else:
        with self.load(url, referer) as f:
          data = f.read(4096)
          while data:
            data = f.read(4096)
        load_finish_callback()
      with self.lock:
        self.loaded_resources.add(url)
      print("resource", url)
    
    tasks = []
    for resource in loading_resources:
      tasks.append({
        "run": lambda r=resource: load_resource(r)
      })
      
    executor_.queue_multiple(tasks)

      

def load_action(name):
  action_file_path = name
  if os.path.abspath(action_file_path) != action_file_path and not action_file_path.endswith('.json'):
    action_file_path = './actor_actions/' + action_file_path + '.json'

  with open(action_file_path, 'r') as af:
    action = json.load(af)
  result = {
    "tasks": {}
  }
  for element in action:
    if element.get('type') == 'properties':
      result.update(element['value'])
    elif element.get('type') == 'task':
      name = element['name']
      if "$" in name:
        print("invald name", name)
        return
      result['tasks'][name] = element
    else:
      return
  return result

def check_age(action):
  safe_age = action.get("safe_age", 0)
  if (safe_age > args_.age):
    return
  proper_age = action.get("proper_age", [0, 3000])
  if not args_.ignore_proper_age:
    if args_.age < proper_age[0]:
      return
    if args_.age > proper_age[1]:
      return
  return True

class Actor:
  def act(self, action):
    print(action)

def play_in_action(action):
  if not check_age(action):
    print("Not doing it at", args_.age, "years old")
    return
  executor_tasks = {}
  lock = threading.Lock()
  for name in action['tasks'].keys():
    anchor_name = name + "$end"
    end_anchor =  {
      "name": anchor_name,
      "dependants": [],
      "waiting": 1
    }
    executor_tasks[anchor_name] = end_anchor
    anchor_name = name + "$start"
    executor_tasks[anchor_name] = {
      "name": anchor_name,
      "dependants": [end_anchor],
      "waiting": 0
    }
    
    
  for name, task in action['tasks'].items():
    start_anchor = executor_tasks[name + "$start"]
    end_anchor = executor_tasks[name + "$end"]
    for before in task['before']:
      other_anchor = executor_tasks[before + "$start"]
      other_anchor["waiting"] = other_anchor["waiting"] + 1
      end_anchor["dependants"].append(other_anchor)

    for after in task['after']:
      other_anchor = executor_tasks[before + "$end"]
      start_anchor["waiting"] = start_anchor["waiting"] + 1
      other_anchor["dependants"].append(start_anchor)

    actions = task['actions']
    actor = Actor()
    def build_run(actor, action):
      def result():
        return actor.act(action)
      return result
    previous_action_task = start_anchor
    delay = task.get("delay_between_actions", None)
    for i in range(len(actions)):
      action_name = name + "$" + str(i)
      action_task = {
        "name": anchor_name,
        "dependants": [],
        "run": build_run(actor, actions[i]),
        "delay": delay,
        "waiting": 1
      }
      if previous_action_task == start_anchor:
        del action_task["delay"]
      previous_action_task["dependants"].append(action_task)
      executor_tasks[action_name] = action_task
      previous_action_task = action_task
    if previous_action_task != start_anchor:
      previous_action_task["dependants"].append(end_anchor)
      end_anchor["waiting"] = end_anchor["waiting"] + 1



  
  executor_.queue_multiple(executor_tasks.values())

action = load_action(args_.action)
play_in_action(action)

#browser = Browser()


#browser.load_webpage("http://www.bing.com/news/search", callback = lambda : print(browser.links))
