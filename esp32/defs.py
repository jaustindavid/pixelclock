import sys

SIDE = 16


def debug(*args, **kwargs):
  print(*args, **kwargs, file=sys.stderr)


def listr(stuff: list[any]) -> str:
  return f"[{', '.join([str(i) for i in stuff])}]"


# returns a value hard-constrained to the square boundary
def constrain(value):
  if value < 0:
    return 0
  if value > SIDE - 1:
    return SIDE -1 
  return value


def map_basic(x, in_min, in_max, out_min, out_max):
  """
  Maps a value from one range to another.

  Args:
    x: The input value to be mapped.
    in_min: The minimum value in the input range.
    in_max: The maximum value in the input range.
    out_min: The minimum value in the output range.
    out_max: The maximum value in the output range.

  Returns:
    The mapped value within the output range.
  """

  # Avoid zero division errors
  if in_min == in_max:
    return out_min

  # Perform linear mapping
  slope = (out_max - out_min) / (in_max - in_min)
  mapped_value = slope * (x - in_min) + out_min

  # Clamp output to the defined range
  return max(out_min, min(out_max, mapped_value))
