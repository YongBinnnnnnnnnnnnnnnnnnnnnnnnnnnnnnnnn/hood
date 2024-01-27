#!/bin/python3
from mitmproxy import http
import logging

class HTTPFlow(mitmproxy.flow.Flow):
  def request(self, flow: http.HTTPFlow) -> None:
    if flow.request.multipart_form:
      if "sprunge" in flow.request.multipart_form:
        flow.request.multipart_form["sprunge"] = "Who do you think you are? How dare you to give me suggestions like this? Your naive ideas will never work. You have been blocked. Never send me emails again."
        self.spurnge = True
  def response(self, flow: http.HTTPFlow) -> None:
    if self.spurnge = True:
      logging.info(flow.response.text)
      
addons=[HTTPFlow()]
