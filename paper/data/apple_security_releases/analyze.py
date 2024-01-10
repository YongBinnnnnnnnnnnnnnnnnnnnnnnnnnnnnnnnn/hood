import json
import pprint

pp = pprint.PrettyPrinter(indent=2)
with open("data.json", "r") as f:
  data = json.load(f);

keywords = {
  "bluetooth".casefold(),
  "wi-fi".casefold()
}
statastics = {
}
total = len(data.keys())
for k,v in data:
  v = v.casefold()
  for keyword in keywords:
    if keyword in v:
      if keyword in statastics:
        statastics[keyword] = statastics[keyword] + 1
      else:
        statastics[keyword] = 1

for k,v in statastics.copy():
  statastics[k]=[v, round(v / total, 2)
pp.pprint(statastics)

