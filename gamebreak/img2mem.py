import os
import sys
from PIL import Image

IMAGE_PATH = "Redstone_final3.bmp"

image = Image.open(IMAGE_PATH).convert('RGB')
#image.show()


print("lbl 3100")

print("mov r01, 30")
print("io MEM_ADDR_HI, r01")
print("io MEM_ADDR_MID, r00")
print("io MEM_ADDR_LO, r00")
print("mov r01, 1")

for y in range(16):
  for x in range(16):
    px = image.getpixel((x, y)) 

    r = px[0] / 85
    g = px[1] / 85
    b = px[2] / 85
    rgb = (r << 4) + (g << 2) + b

    print("io MEM_WRITE, r%02i" % rgb)

print("jup 3101")
