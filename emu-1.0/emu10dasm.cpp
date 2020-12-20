
#include <iostream>
#include <fstream> // or something

#include "Disassembler.h"
#include "Tape.h"

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cerr << "USAGE: emu10vm <tape>" << std::endl;
    return 1;
  }

  std::ifstream tapeFile;
  tapeFile.open(argv[1], std::ios_base::binary);
  
  if (!tapeFile) {
    std::cerr << "Unable to open tape: " << argv[1] << std::endl;
    return 1;
  }
  
  //tapeFile.seekg(0, std::ios::end);
  
  Tape tape(tapeFile, false);
  tapeFile.close();
  
  while (!tape.eof())
  {
    printf("%05zu ", tape.getPos());
    std::cout << disassemble(tape.read()) << std::endl;
    
    tape.advance(Tape::Direction::DOWN);
  }

  std::cerr << "Done!" << std::endl;
  
  return 0;
}

