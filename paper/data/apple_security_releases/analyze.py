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
  match_pattern = re.findall(r'\n[a-z\-A-Z]{2,15}\n', v)
  for word in match_pattern:
    word = word.strip()
    count = statastics.get(word,0)
    statastics[word] = count + 1


bar_width = 30
def draw_bar(word, percentage):
  length = round(bar_width*percentage)
  print(word + ": |" + "█"*length + " "*(bar_width - length) + "|")


max_count = max(statastics.values())
print(max_count )
for k, v in statastics.copy().items():
  if v < 20:
    del statastics[k]
    continue
  draw_bar(k, v/max_count)
  #statastics[k]=[v, round(v / total, 2)]
#pp.pprint(statastics)

