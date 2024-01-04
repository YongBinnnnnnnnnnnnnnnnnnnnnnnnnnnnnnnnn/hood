#!/usr/bin/python3 -B
import asyncio
import sys
def load_hood_name_service():
  import importlib.util
  import sys
  spec = importlib.util.spec_from_file_location("hood-name-service.py",  "/usr/local/lib/hood/hood-name-service.py")
  module = importlib.util.module_from_spec(spec)
  sys.modules["hood_name_service"] = module
  spec.loader.exec_module(module)
  return module

HoodResolve = load_hood_name_service().HoodResolve

async def main():
    if len(sys.argv) < 2:
      return
    print(await HoodResolve(sys.argv[1]))


asyncio.run(main())