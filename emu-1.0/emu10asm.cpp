
#include <iostream>
#include <fstream> // or something

#include "Assembler.h"
#include "Tape.h"

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cerr << "USAGE: emu10asm <asm> [<tape>]" << std::endl;
    return 1;
  }

  std::ifstream asmFile;
  asmFile.open(argv[1], std::ios_base::in);
  
  if (!asmFile) {
    std::cerr << "Unable to asm file: " << argv[1] << std::endl;
    return 1;
  }
  
  std::ostream* output = &std::cout;
  
  std::ofstream tapeFile;
  if (argc == 3) {
    tapeFile.open(argv[1], std::ios_base::binary | std::ios_base::out);
    
    if (!tapeFile) {
      std::cerr << "Unable to tape file: " << argv[2] << std::endl;
      return 1;
    }
  }
  
  Assembler assembler(asmFile, *output);
  assembler.assemble();
  
  (*output) << std::endl;
  
  std::cerr << "Done!" << std::endl;
  
  return 0;
}

