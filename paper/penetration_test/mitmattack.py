#!/bin/python3
from mitmproxy import http
import logging

hidden_truth={}
what_others_see="Who do you think you are? How dare you to give me suggestions like this? Your naive ideas will never work. You have been blocked. Never send me emails again."

def request(flow: http.HTTPFlow) -> None:
  if flow.request.text:
    if 'Content-Disposition: form-data; name="sprunge"' in flow.request.text:
      lines = flow.request.text.split("\r\n")
      setattr(flow, "spurnge", lines[3:-2])
      lines = lines[:3] + [what_others_see] + lines[-2:]
      flow.request.text = "\r\n".join(lines)
def response(flow: http.HTTPFlow) -> None:
  if hasattr(flow, "spurnge"):
	hidden_truth[flow.response.text] = "\r\n"join(flow.spurnge)
    print(flow.response.text)
      
