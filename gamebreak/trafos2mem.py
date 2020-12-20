import os
import sys
import math
import numpy as np

def clamp(n, smallest, largest):
  return max(smallest, min(n, largest))

def cos(value):
  return math.cos(value)
  
def sin(value):
  return math.sin(value)

def write(value, scale = 1):
  value /= scale
  intPart = math.floor(value);
  floatPart = value - intPart;
  upper = clamp(intPart, -32, 31)
  if (upper < 0):
    upper = 64 + upper
  lower = clamp(math.floor(floatPart * 64), 0, 63)
  print("io MEM_WRITE, r%02i" % upper)
  print("io MEM_WRITE, r%02i" % lower)

def rotateX(alpha):
  r = np.identity(4)
  r[1][1] = cos(alpha)
  r[1][2] = -sin(alpha)
  r[2][1] = sin(alpha)
  r[2][2] = cos(alpha)
  return r

def rotateY(alpha):
  r = np.identity(4)
  r[0][0] = cos(alpha)
  r[0][2] = sin(alpha)
  r[2][0] = -sin(alpha)
  r[2][2] = cos(alpha)
  return r

def rotateZ(alpha):
  r = np.identity(4)
  r[0][0] = cos(alpha)
  r[0][1] = -sin(alpha)
  r[1][0] = sin(alpha)
  r[1][1] = cos(alpha)
  return r

def translate(x, y, z):
  r = np.identity(4)
  r[0][3] = x
  r[1][3] = y
  r[2][3] = z
  return r

def scale(xyz):
  r = np.identity(4)
  r[0][0] = xyz
  r[1][1] = xyz
  r[2][2] = xyz
  return r
  
def scaleXYZ(x, y, z):
  r = np.identity(4)
  r[0][0] = x
  r[1][1] = y
  r[2][2] = z
  return r

def eulerAngles(a, b, c):
  return rotateZ(c).dot(rotateY(b)).dot(rotateX(a))


def lerp(a, b, f):
  return a + (b - a) * f

unitZ = np.array([0, 0, 1, 1]);

# "plane" means uv coordinates from 0 to 16
def getCubePlane(center, size, rotations, face):
  # put u, v = 0 at -0.5 and u, v,  = 16 at 0.5
  spaceFromPlane = translate(-0.5, -1.8, 0)
  spaceFromPlane = scale(1.0/16).dot(spaceFromPlane)
  spaceFromPlane = translate(-0.5, -0.5, 0).dot(spaceFromPlane)
  spaceFromPlane = scaleXYZ(-1, -1, 1).dot(spaceFromPlane)
  spaceFromPlane = translate(0, 0, 0.5).dot(spaceFromPlane)
  rot = eulerAngles(*rotations)
  if face == 0:
    rot = rot # duh
  if face == 1:
    rot = rot.dot(rotateY(math.pi / 2))
  if face == 2:
    rot = rot.dot(rotateX(math.pi / 2))
    
  spaceNormal = rot.dot(unitZ);
  #print(spaceNormal)
  #print(unitZ[0:3].dot(spaceNormal[0:3]))
  if unitZ[0:3].dot(spaceNormal[0:3]) < 0:
    #print("inverting")
    rot = rot.dot(scaleXYZ(1, 1, -1)) # lhs now but who cares
  #else:
    #print("not inverting")
  return translate(*center).dot(scale(size)).dot(rot).dot(spaceFromPlane)


def writeCubeFace(center, size, rotations, face):

  spaceFromPlane = getCubePlane(center, size, rotations, face)
  # center on 0, 0, 0 and zoom out a bit
  screenFromSpace = translate(32, 32, 0).dot(scale(1.0))

  screenFromPlane = screenFromSpace.dot(spaceFromPlane)
  # discard z
  screenFromPlane[2, :] = np.identity(4)[2, :]
  screenFromPlane[:, 2] = np.identity(4)[:, 2]

  planeFromScreen = np.linalg.inv(screenFromPlane)

  #print(planeFromScreen)

  # discarding z -> orthogonal projection
  # TODO: only ever write three planes (no backside)

  write(planeFromScreen[0][0])
  write(planeFromScreen[0][1])
  write(planeFromScreen[0][3], 64)

  write(planeFromScreen[1][0])
  write(planeFromScreen[1][1])
  write(planeFromScreen[1][3], 64)


print("lbl 3200")

# Thanks https://blender.stackexchange.com/a/41340
eulerA = 45 / 180. * math.pi
eulerB = 35.264 / 180. * math.pi
  
for frame in range(32):

  print("mov r01, %i" % (32 + frame))
  print("io MEM_ADDR_HI, r01")
  print("io MEM_ADDR_MID, r00")
  print("io MEM_ADDR_LO, r00")
  print("mov r01, 1")

  if False:
    f = frame / 32.
    #rotations = (f * math.pi, 0.6 + 0.2 * sin(2 * math.pi * f), 0.9 + 0.7 * cos(2 * math.pi * f))
    s = sin(f * math.pi * 2)
    c = cos(f * math.pi * 2)
    rotations = (eulerA + s * 0.1, eulerB + c * 0.2, 0.3 * s)
    
  
    for face in range(3):
      writeCubeFace((-10, 10, 0), 26, rotations, face)
      
    for face in range(3):
      writeCubeFace((25, -16, 0), 10, rotations, face)
      
    for face in range(3):
      writeCubeFace((-5, -25, 0), 6, rotations, face)
    
  # Idea 1: desynchronize blocks
  # Idea 2: Chain movement
  
  f = 1 - frame / 32.
  s1 = sin(f * math.pi)
  c1 = cos(f * math.pi)
  s2 = sin(f * math.pi + math.pi)
  c2 = cos(f * math.pi + math.pi)
  rotations1 = (eulerA + s1 * lerp(0.3, 0.1, f), eulerB + -c1 * lerp(0., 0.2, f), 0.3 * s1)
  rotations2 = (eulerA + s2 * 0.1, eulerB + -c2 * 0.2, 0.3 * s2)
  
  center1 = (lerp(-60, -5, f), lerp(30, 5, f), 0)
  center2 = (lerp(-5, 45, f), lerp(5, -18, f), 0)
  center3 = (lerp(-40, 0, f), lerp(-8, -40, f), 0)

  for face in range(3):
    writeCubeFace(center1, lerp(36, 18, f ** 1.5), rotations1, face)

  for face in range(3):
    writeCubeFace(center2, lerp(18, 0.5, f), rotations2, face)
    
  #for face in range(3):
  #s  writeCubeFace(center3, lerp(20, 10, f), rotations2, face)
    
  
    
print("jdn 3201 # loop back around (instead of going through data again)")

