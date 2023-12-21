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

def load_hood_name_service():
  import importlib.util
  import sys
  spec = importlib.util.spec_from_file_location("hood-name-service.py",  "/usr/local/lib/hood/hood-name-service.py")
  module = importlib.util.module_from_spec(spec)
  sys.modules["hood_name_service"] = module
  spec.loader.exec_module(module)
  return module
  
load_hood_name_service().HookGetAddrInfo()

parser = argparse.ArgumentParser(
  prog="hood-timesync",
  description="Behavioral simuluation tool for hood firewall.",
  epilog="https://github.com/YongBinnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn/hood",
)

parser.add_argument("--action", default="browse_bing_news", type=str, help="Action to act.")
parser.add_argument("--age", default=13, type=int, help="Age to act.")
parser.add_argument("--thread-pool-size", default=16, type=int, help="Age to act.")
parser.add_argument("--ignore-proper-age", default=False, type=bool, help="Follow the proper age range.")
parser.add_argument("--debug", default=False, type=bool, help="Follow the proper age range.")
args_ = parser.parse_args()

if args_.debug:
  LOG_DEBUG = print
else:
  def LOG_DEBUG(*args, **kwargs):
    pass


#TODO: HAR or pcap replay

default_accept_ = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7"
image_accept_ = "image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8"
css_accept_ = "text/css,*/*;q=0.8"
any_accept_ = "*/*;q=0.8"

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
  new_delayed_task_event = threading.Event()
  task_queue = []
  delayed_queue = []
  already_delayed_tasks = []
  queue_lock = threading.Lock()
  thread_local = threading.local()
  delay_precision = 0.1
  deleted = False

  def __init__(self, pool_size=8):
    for _ in range(pool_size):
      self.pool.append(threading.Thread(target=self.worker).start())
    threading.Thread(target=self.delay_scheduler).start()

  def __del__(self):
    self.deleted = True
    self.new_task_event.set()
    self.new_delayed_task_event.set()

  def queue_one(self, task):
    with self.queue_lock:
      self.task_queue.append(task)
    self.new_task_event.set()
        
  def queue_multiple(self, task_list):
    task_list = list(filter(lambda task:not task.get("waiting", 0), task_list))
    with self.queue_lock:
      self.task_queue = self.task_queue + task_list
      alert_times = min(len(self.pool), len(self.task_queue))
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
    self.new_delayed_task_event.set()

  def delay_scheduler(self):
    while not self.deleted:
      if not self.delayed_queue:
        self.new_delayed_task_event.wait()
        self.new_delayed_task_event.clear()
      time.sleep(self.delay_precision)
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
    while not self.deleted:
      self.new_task_event.wait()
      if self.deleted:
        break
      self.new_task_event.clear()
      while self.task_queue:
        self.__run(self.get())
        if self.deleted:
          break

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
    
executor_ = Executor(pool_size = args_.thread_pool_size)

class HoodHTMLParser(HTMLParser):
  #useless_tags = set(['meta', 'div', 'i', 'span', 'p', 'style', 'map', 'area', 'b', 'form', 'title', 'noscript', 'li', 'ul', 'input', 'html', 'head', 'body', 'textarea'])
  useless_tags = None
  tags_with_src = set(['script', 'img', 'audio', 'video', 'iframe'])
  tags_with_href = set(['a', 'link'])
  load_rels = set(['icon', 'stylesheet', 'preload'])
  image_tags = set(['link:image', 'img', 'link:icon'])

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
            if attr[1] != 'preload':
              tag = tag + ':' + attr[1]
          if attr[0] == 'as':
              tag = tag + ':' + attr[1]
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

  def load(self, url, headers = None, referer = None, accept = None):
    try:
      if headers is None:
        headers = headers_.copy()
      headers["Connection"] : "close";
      if url.startswith('http://'):
        del headers["Upgrade-Insecure-Requests"]
      if referer:
        headers["Referer"] = referer
      if accept:
        headers["Accept"] = accept
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
    with self.load(url, referer = referer) as f:
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
    blockers = []
    def load_resource(resource):
      nonlocal countdown_lock
      (tag, url) = resource
      with countdown_lock:
        blockers.append(url)
      LOG_DEBUG("loading resource", url)
      if resource[0] == 'iframe':
        self.load_webpage(url, iframe = True, callback = load_finish_callback)
      else:
        accept = None
        if tag in HoodHTMLParser.image_tags:
          accept = image_accept_
        elif tag == 'link:stylesheet':
          accept = css_accept_
        elif tag == 'script':
          accept = any_accept_
        response = self.load(url, referer=referer, accept=accept)
        if response:
          with response as f:
            loaded = False
            with self.lock:
              if url not in self.loaded_resources:
                self.loaded_resources.add(url)
              else:
                loaded = True
            if not loaded:
              try:
                while len(f.read(4096, 1)) == 4096:
                  pass
              except:
                pass
        load_finish_callback()
      LOG_DEBUG("loaded resource", url, countdown)
      with countdown_lock:
        blockers.remove(url)
        LOG_DEBUG("blockers", blockers)
    
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
      properties = ["age", "proper_age"]
      for property in properties:
        if property in element:
          result[property] = element[property]
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
  browser = None
  def act(self, action):
    print(action)
    if action.startswith("hood:"):
      return self.eval_hood_action_script(action[5:])
  
  def eval_hood_action_script(self, action):
    if action.startswith("browser:"):
      action = action[8:]
      if action.startswith("goto:"):
        url = action[5:]
        if not self.browser:
          self.browser = Browser()
        load_finish_event = threading.Event()
        if url == "random_link":
          link_count = len(self.browser.links)
          if link_count == 0:
            print("current page has no links")
            return
          url = self.browser.links[random.randint(0, link_count - 1)]
        self.browser.load_webpage(url, callback=lambda:load_finish_event.set())
        load_finish_event.wait(timeout=20)
        print("finished to load", url, "in browser")
    elif action.startswith("loop:"):
      action = action[5:]
      condition_end = action.index(":")
      condition = self.compile_condition(action[:condition_end])
      action = action[condition_end + 1:]
      while condition():
        self.act(action)
  def compile_condition(self, text):
    subconditions = text.split(',')
    decision_condition = lambda:True
    other_conditions = []
    for condition in subconditions:
      if condition.startswith("exit_chance="):
        chance = float(condition[12:])
        decision_condition = lambda: random.random() > chance
      elif condition.startswith("delay="):
        ranges = list(map(float, condition[6:].split("-")))
        def compile(min, max):
          return lambda: time.sleep(random.uniform(min, max))
        other_conditions.append(compile(*ranges))
    def compiled():
      for condition in other_conditions:
        condition()
      return decision_condition()
    return compiled


def play_in_action(action):
  if not check_age(action):
    print("Not doing it at", args_.age, "years old")
    return
  action_done_event = threading.Event()
  action_done_task = {
    "run": lambda: action_done_event.set(),
    "waiting": len(action['tasks'].keys())
  }
  executor_tasks = {}
  for name in action['tasks'].keys():
    anchor_name = name + "$end"
    end_anchor =  {
      "name": anchor_name,
      "dependants": [action_done_task],
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
      other_anchor = executor_tasks[after + "$end"]
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
  action_done_event.wait()
action = load_action(args_.action)
play_in_action(action)
print("Finished acting")
executor_.__del__()

for i in range(args_.thread_pool_size * 2):
  time.sleep(0.01)
os._exit(os.EX_OK)
