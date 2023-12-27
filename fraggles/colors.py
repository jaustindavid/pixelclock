# 16 is basically the lowest value visible when
# brightness is turned down

COLOR = {
  '0': (0,0,0),
  '1': (4,4,4),
  '2': (8,8,8),
  '3': (12,12,12),
  '4': (16,16,16),
  '5': (20,20,20),
  '.': (0, 4, 0),
  'o': (4, 4, 4),
  'w': (16, 16, 16),
  'O': (32, 32, 32),
  'R': (128, 0, 0),
  'r': (16, 0, 0),
  'G': (0, 64, 0),
  'g': (0, 16, 0),
  'B': (0, 0, 128),
  'b': (0, 0, 16),
  'Y': (128, 128, 0),
  'L': (16, 64, 0),
  'h': (32, 16, 0),
  'W': (128, 128, 128),
  ' ': (0, 0, 0)
}


if __name__ == "__main__":
  from matrix import Matrix
  from pixel import Pixel
  import defs
  sandbox = []
  x = 0
  y = 0
  for color in COLOR.keys():
    sandbox.append(Pixel(x,y,color))
    x += 1
    if x >= defs.SIDE:
      x = 0
      y += 1
  print(Matrix.to_str(16, sandbox))
  matrix = Matrix(16, sandbox)
  matrix.show()
