

// C code?

unpackDataToRam();

word numPlanes = ...;

for (word y = 0; y < 64; ++y)
  for (word x = 0; x < 64; ++x)
  {
    for (word planeIndex = 0; planeIndex < numPlanes; ++x)
    {
      matmul(x, y, planeMatsPtr + MAT_SIZE * planeIndex); // MATSIZE = 12?
      if (upperU != 0)
        continue;
      if (upperV != 0)
        continue;
      // midU and midV is between 0 and 64 anyway
      // could do bilinear with lowerU/lowerV but colors...
      
      // Alternatively just scroll the tape...
      selectRam(midV);
      color = loadFromRamLine(midU);
      jmp nextX;
    }
        
    // comets -> 0.1 * square(x - ...
    
    // stars
  }

void matmul(word x, word y, word matPtr)
{
  word upperU, midU, lowerU; // r20-r22
  // All multiplies signed?
  upperU = (matPtr[0] * x) >> 6;
  midU = (matPtr[0] * x) >> 0;
  midU += (matPtr[1] * x) >> 6;
  lowerU += (matPtr[1] * x) >> 0;
  
  upperU += (matPtr[2] * y) >> 6;
  midU += (matPtr[2] * y) >> 0;
  midU += (matPtr[3] * y) >> 6;
  lowerU += (matPtr[3] * y) >> 0;
  
  midU += matPtr[4];
  lowerU += matPtr[5];

  // repeat for v
}



