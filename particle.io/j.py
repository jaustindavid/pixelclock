#! python3
import json

with open("json.txt", "rt") as f:
  data = json.load(f)
  print(data.keys())
  print(data['weather'][0]['icon'])
  print(json.dumps(data['main']['feels_like']))
