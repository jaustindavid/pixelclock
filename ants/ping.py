import subprocess
from timer import Timer
from datetime import timedelta

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
  for line in lines:
    if "time=" in line:
      rtt = float(line.split("time=")[1].split("ms")[0])
      break
  if rtt < 50:
    return 8
  elif rtt < 100:
    return 7
  elif rtt < 150:
    return 6
  elif rtt < 200:
    return 5
  elif rtt < 250:
    return 4
  elif rtt < 300:
    return 3
  elif rtt < 400:
    return 2
  else:
    return 1


if __name__ == "__main__":
  minute = Timer(timedelta(seconds=60))
  outfile = "ping.txt"
  while True:
    latency_score = ping_latency("google.com")
    print(f"Latency score: {latency_score}")
    with open(outfile, "wt") as txt:
      txt.write(f"{latency_score}")
    if latency_score >= 7:
      minute.wait()
