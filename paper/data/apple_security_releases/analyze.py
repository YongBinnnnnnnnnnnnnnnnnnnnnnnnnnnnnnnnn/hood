import json
import pprint
import re

pp = pprint.PrettyPrinter(indent=2)
with open("data.json", "r", encoding='utf-8') as f:
  data = json.load(f);

statastics = {
}
for k, v in data.items():
  #print(text_string)
  match_pattern = re.findall(r'\n[ a-z\-A-Z]{2,15}\n', v)
  for word in match_pattern:
    word = word.strip()
    count = statastics.get(word,0)
    statastics[word] = count + 1
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

