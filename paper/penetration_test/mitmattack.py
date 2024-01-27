#!/bin/python3
from mitmproxy import http

def request(flow: http.HTTPFlow) -> None:
  if flow.request.urlencoded_form:
    flow.request.urlencoded_form["sprunge"] = "Who do you think you are? How dare you to give me suggestions like this? Your naive ideas will never work. You have been blocked. Never send me emails again."
