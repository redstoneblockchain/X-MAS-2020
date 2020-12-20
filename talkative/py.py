#!/usr/bin/env python3

import os
import sys
import socket
import time
from math import sqrt
import math
import operator

HOST = "challs.xmas.htsp.ro"
PORT = 5100
CHUNK = 10000000

CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ +-*/<=>()[]{}#$_?|^&!~,.:\n "

chatLog = ''

def sendData(msg):
  global chatLog
  #print("Sending " + msg)
  #chatLog += msg
  sent = s.send(msg)
  if sent != len(msg):
    raise RuntimeError("socket connection broken")
  time.sleep(0.5)

def recvData(retries=4):
  global chatLog
  data = s.recv(CHUNK)
  chatLog += data
  print(data)
  if data == '':
    if retries > 0:
      time.sleep(2)
      recvData(retries-1)
    else:
      raise RuntimeError("I can't hear you")
  time.sleep(0.5)
  return data



s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

msg = [34, 22, 10, 28, 18, 36, 22, 18, 16, 17, 29, 36, 11, 14, 36, 11, 14, 29, 29, 14, 27, 36, 29, 17, 10, 23, 36, 33, 38, 22, 10, 28, 36, 21, 24, 21, 24, 21, 24, 21];

sendData("".join([chr(b) for b in msg]));


response = recvData()

print(response)

print([ord(r) for r in response])

decoded = "".join([CHARSET[ord(r)] for r in response])

print(decoded)
