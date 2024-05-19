import random

class Instruction:
  def __init__(self, line):
    self.line = line
    self.tokens = line.split()
    self.tc = 0 # token counter

  def __str__(self):
    return str(self.tokens)

  def startswith(self, token):
    return self.tokens[0] == token

  # returns the number of leading spaces
  def depth(self):
    return len(self.line) - len(self.line.lstrip())

  def next_token(self):
    if self.tokens:
      return self.tokens.pop(0)
    return None


# if on food
#   return
# if step_toward adjacent food
#   return
# if step_toward nearish food
#   return
# wander


class Parser:
  def __init__(self, filename):
    self.instructions = self.read(filename)


  # consumes the contents of the named file
  def read(self, filename):
    with open(filename, 'r') as f:
      content = f.read()
    return content.splitlines()


  def execute(self, instruction) -> bool:
    print(f"got {instruction}")
    token = instruction.next_token()
    if token == "on_food":
      print(f"ant ON FOOD?")
      return random.choice([True, False])
    elif token == "step_toward":
      print(f"stepping toward {instruction}")
      return random.choice([True, False])
    elif token == "wander":
      print(f"ant.wander()")
    else:
      print(f"TBD ({token}: {instruction})")
    return random.choice([True, False])


  # iterate once
  def step(self):
    print(f"stepping through: {self.instructions}")
    self.pc = 0
    self.depth = 0
    while True:
      if self.pc >= len(self.instructions):
        print(">> EOF")
        break
      instruction = Instruction(self.instructions[self.pc])
      print(f">> reading: {instruction}")
      print(f">> depth = {self.depth}")
      if instruction.depth() == self.depth:
        print(f">> RUNNING {instruction}")
        if instruction.startswith("if"):
          instruction.next_token()
          print(">> IF")
          if self.execute(instruction):
            print(">> TRUE")
            self.depth += 2
          else:
            print(">> FALSE")
        elif instruction.startswith("return"):
          print(">> RETURN")
          break
        else:
          self.execute(instruction)
      else:
        print(f">> skipping nested part: {instruction}")
      self.pc += 1


if __name__ == "__main__":
  parser = Parser("clock.ant")
  parser.step()
