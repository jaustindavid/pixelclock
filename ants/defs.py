SIDE = 16

# returns a value hard-constrained to the square boundary
def constrain(value):
  if value < 0:
    return 0
  if value > SIDE - 1:
    return SIDE -1 
  return value
