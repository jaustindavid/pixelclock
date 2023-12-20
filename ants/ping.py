from typing import List, Tuple
import subprocess
from time import sleep
from timer import Timer
from datetime import datetime, timedelta
import json
import mproc


''' asked bard to estimate internet latency, gave me this '''
def ping_latency(server_address):
  """
  Measures the latency to a server using ping and returns an estimated score.

  Args:
    server_address: The IP address or hostname of the server to ping.

  Returns:
    An integer between 1 and 8, where 1 is the worst and 8 is the best.
  """
  ping_process = subprocess.Popen(["ping", "-c", "3", server_address], stdout=subprocess.PIPE)
  output, error = ping_process.communicate()
  if error:
    return 1  # Assume worst case if ping fails
  lines = output.decode().splitlines()
  rtt = 999
  for line in lines:
    if "time=" in line:
      rtt = float(line.split("time=")[1].split("ms")[0])
      break
  if rtt < 50:
    score = 8
  elif rtt < 100:
    score = 7
  elif rtt < 150:
    score = 6
  elif rtt < 200:
    score = 5
  elif rtt < 250:
    score = 4
  elif rtt < 300:
    score = 3
  elif rtt < 400:
    score = 2
  else:
    score = 1

  return (rtt, score)


def read(filename: str) -> List[Tuple]:
  try:
    data = list()
    with open(filename, "rt") as f:
      data = json.load(f)
    if isinstance(data, list):
      return data
  except:
    pass
  return []


# returns the newest (first) tuple in data
def get_score():
  data = read("ping.txt")
  if len(data) > 1:
    return data[0]
  return (1, 999, 0)


def write(filename: str, data: List[Tuple]):
  with open(filename, "wt") as txt:
    txt.write(json.dumps(data))


def ping_forever():
  minute = Timer(timedelta(seconds=15))
  outfile = "ping.txt"
  data = read(outfile)
  while True:
    rtt, score = ping_latency("google.com")
    timestamp = int(datetime.now().timestamp())
    datum = (rtt, score, timestamp)
    data.insert(0, datum)
    if len(data) > 8:
      data.pop()
    # print(f"Latency score: {score}")
    print(get_score())
    write(outfile, data)
    if score >= 7:
      minute.wait()
    else:
      sleep(5) # simple ratelimit


if __name__ == "__main__":
  ping_forever()
