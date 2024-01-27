#!/bin/python3
from mitmproxy import http

def request(flow: http.HTTPFlow) -> None:
  if flow.request.urlencoded_form:
    print(flow.request.urlencoded_form)
