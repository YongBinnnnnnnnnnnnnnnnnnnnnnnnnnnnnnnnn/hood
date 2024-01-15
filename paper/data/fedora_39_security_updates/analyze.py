#!/usr/bin/python3
import json
import re

with open("data.json", "r", encoding='utf-8') as f:
  data = json.load(f);

statastics = {
}
for l in data:
  l=l[0]
  print("-".join(l.split("-")[0:-2]))
  count = statastics.get(l,0)
  statastics[l] = count + 1
statastics["Wi-Fi + Bluetooth"] = statastics["Wi-Fi"] + statastics["Bluetooth"]
del statastics["Security"]
statastics = {k: v for k, v in reversed(sorted(statastics.items(), key=lambda item: item[1]))}

bar_width = 58
word_width = 18
def draw_bar(word, value, max):
  length = round(bar_width*value/max)
  if len(word) < word_width:
    word = word+" "*(word_width - len(word))
  print(word + ": |" + "█"*length + " "*(bar_width - length) + "| " + str(value)  )

max_count = max(statastics.values())
print("Word" + " "*(word_width - 4) + " " + " "*(bar_width+2) + "  Times" )
draw_count = 20
for k, v in statastics.copy().items():
  draw_bar(k, v, max_count)
  draw_count = draw_count - 1
  if draw_count == 0:
    break
  #statastics[k]=[v, round(v / total, 2)]
#pp.pprint(statastics)

