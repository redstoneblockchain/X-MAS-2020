# Conventions (after initialization):
# r01-r09 -> function parameters (not preserved)
# r10-r19 -> function returns
# r20-r29 -> scratch space
# r30-r39 -> globals, do not touch during function calls
# r40-r49 -> constants
# r63 return label index

#
# Initialization
#
mov r01, 0
lbl 1
st [r01], r01
addi r01, r01, 1
cmpeq r01, 0
-jup 1
# rxx now has value xx
# this is useful because memory writes below cannot use immediates

# Load texture
#mov r01, 30 # start of texture data
#io MEM_ADDR_HI, r01
#io MEM_ADDR_MID, r00
#io MEM_ADDR_LO, r00
#mov r01, 1
jdn 3100 # call write texture data
lbl 3101

# Load matrices
#mov r01, 32
#io MEM_ADDR_HI, r01
#io MEM_ADDR_MID, r00
#io MEM_ADDR_LO, r00
#mov r01, 1
jdn 3200 # call write matrix data
lbl 3201

# set constants now
mov r40, 30 # start of texture data
mov r42, 32 # start of matrix data
mov r43, 6 # number of matrices
mov r44, 12 # size of each matrix

mov r32, 0 # frame number
mov r33, 0 # cycle counter

#
# Drawing routine
#
lbl 10

mov r35, 0
# loop y
lbl 100

  mov r34, 0
  # loop x
  lbl 101

    mov r36, 0
    # loop matrix
    lbl 102

      mov r01, r34
      mov r02, r35
      mov r03, r36
      # call matrix mult
      jdn 200
      lbl 201

      # We have u in r10-r12 and v in r13-r15
      # require hi == 0
      #cmpeq r10, 0
      #-jdn 103
      #cmpeq r13, 0
      #-jdn 103
      # ok, but require hi == 0 +-1
      # ok, still not good enough, but +- 6 is fine
      cmpsl r10, 6
      -jdn 103
      cmpsg r10, 58
      -jdn 103
      cmpsl r13, 6
      -jdn 103
      cmpsg r13, 58
      -jdn 103
      # require 0 <= mid < 16
      cmpul r11, 16
      -jdn 103
      cmpul r11, 0
      +jdn 103
      cmpul r14, 16
      -jdn 103
      cmpul r14, 0
      +jdn 103

      mov r01, r14 # will be mem mid
      mov r02, r14 # will be mem low
      mov r16, 16
      fmu06 r01, r16
      fmu00 r02, r16
      add r02, r02, r11 # beware uncaught overflow! Kahan?

      io MEM_ADDR_HI, r40 # start of texture data
      io MEM_ADDR_MID, r01
      io MEM_ADDR_LO, r02
      io r09, MEM_READ

      io GPU_X, r34
      io GPU_Y, r35
      io GPU_DRAW, r09

      jdn 109 # goto done with this pixel


      lbl 103 # continue (next matrix)

    # loop matrix
    addi r36, r36, 1
    cmpne r36, r43 # while (matrix != numMatrices)
    +jup 102

    # clear pixel, background
    io GPU_X, r34
    io GPU_Y, r35
    mov r01, 1 # dark blue
    add r03, r35, r35
    add r03, r03, r34
    add r03, r03, r34
    add r03, r03, r32
    addi r03, r03, 1
    sub r04, r63, r35
    fmu05 r03, r04
    andi r02, r03, 20
    cmpul r02, 2
    +mov r01, 6
    cmpeq r02, 4
    +mov r01, 10
    #andi r02, r34, 15
    #cmpeq r02, 1
    #+andi r02, r35, 1
    #+cmpeq r02, 0
    #+mov r01, 6
    io GPU_DRAW, r01
    
    # TODO: background effect

    # done with this pixel
    lbl 109

  # loop x
  addi r34, r34, 1
  cmpne r34, 0
  +jup 101

# loop y
addi r35, r35, 1
cmpne r35, 0
+jup 100


addi r32, r32, 1 # next frame
cmpeq r32, 0
+addi r33, r33, 1

# timer loop?

io r00, CLOCK_LO_CS, r40 # screen refresh

# goto drawing routine
jup 10



#
# Matrix mult
# Take x in r01, y in r02 and matrix id in r03
# Return u in r10-r12 and v in r13-r15
#
lbl 200

# truncate time to 5 bit and use that page of matrices
#mov r04, 0
andi r04, r32, 31
# every 4th cycle, run backward? quadratic slowdown?
#andi r05, r33, 1
#cmpeq r05, 1
#+cmpul r32, 60
#+addi r32, r32, 1
# don't draw first three planes in last half of going backwards?

add r04, r04, r42 # first matrix data page
io MEM_ADDR_HI, r04

# compute page offset from matrix number in r03
mov r06, r44
mov r07, r44
fmu06 r06, r03
fmu00 r07, r03

io MEM_ADDR_MID, r06
io MEM_ADDR_LO, r07

mov r05, 0 # row number
# for matrix row
lbl 202

  # read matrix row (2 words per entry)
  io r20, MEM_READ
  io r21, MEM_READ
  io r22, MEM_READ
  io r23, MEM_READ
  io r24, MEM_READ
  io r25, MEM_READ

  # compute all parts into r13-r15
  # the first time around they are then moved to r10-12 afterwards

  mov r13, 0
  mov r14, 0
  mov r15, 0
  
  # for overflows in addition of lower parts, consider:
  # a + b >= 64 iff a >= 64 - b = 0 - b (or b >= 64 - a = 0 - a)
  
  # hiU = (matPtr[0] * x) >> 6;
  # midU = (matPtr[0] * x) >> 0;
  # midU += (matPtr[1] * x) >> 6;
  # loU = (matPtr[1] * x) >> 0;
  mov r08, r20
  fms06 r08, r01
  add r13, r13, r08
  
  mov r08, r20
  fmu00 r08, r01
  add r14, r14, r08
  
  mov r08, r21
  fmu06 r08, r01
  # beware overflow!
  sub r09, r00, r08
  cmpul r14, r09 # if this is true, no overflow is happening
  #-addi r13, r13, 1 # could itself overflow but nothing we can do about it
  add r14, r14, r08
  
  mov r08, r21
  fmu00 r08, r01
  add r15, r15, r08

  # hiU += (matPtr[2] * y) >> 6;
  # midU += (matPtr[2] * y) >> 0;
  # midU += (matPtr[3] * y) >> 6;
  # loU += (matPtr[3] * y) >> 0;
  mov r08, r22
  fms06 r08, r02
  add r13, r13, r08 # beware overflow (but we cannot do anything about it)
  
  mov r08, r22
  fmu00 r08, r02
  # handle overflow
  sub r09, r00, r08
  cmpul r14, r09
  #-addi r13, r13, 1
  add r14, r14, r08

  mov r08, r23
  fmu06 r08, r02
  # handle overflow
  sub r09, r00, r08
  cmpul r14, r09
  #-addi r13, r13, 1
  add r14, r14, r08
  
  mov r08, r23
  fms00 r08, r02
  # handle overflow
  sub r09, r00, r08
  cmpul r15, r09
  #-addi r14, r14, 1 # why does this produce garbage??
  cmpfa r00, r00
  -cmpeq r14, 0 # carried all the way?
  #+addi r13, r13, 1
  add r15, r15, r08
    
  # hiU += matPtr[4];
  # midU += matPtr[5];
  add r13, r13, r24 # beware overflow (but we cannot do anything about it)
  
  # handle overflow
  sub r09, r00, r25
  cmpul r14, r09
  -addi r13, r13, 1
  # actual addition
  add r14, r14, r25

  # if first time, move results to u return values
  # the second time, keep them in v return values
  cmpeq r05, 0
  mov r05, 1
  +mov r10, r13
  +mov r11, r14
  +mov r12, r15
  # repeat with v
  +jup 202

# done!
jup 201



#
# Write texture data
#
#lbl 3100
# 16 * 16 texture pixels
#io MEM_WRITE, r42
# ...
#jdn 3101


#lbl 3200
# Matrix data
#io MEM_WRITE, r63
#...
#jdn 3201 # loop back around (instead of going through data again)
