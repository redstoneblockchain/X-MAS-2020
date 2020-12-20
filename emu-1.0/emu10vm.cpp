
#include <iostream>
#include <fstream> // or something
#include <set>

#include "Disassembler.h"
#include "Screen.h"
#include "VM.h"

int main(int argc, char **argv) {
  
  std::ifstream tapeFile;
  //bool enable_dasm = false;
  bool enable_looped = false;
  bool enable_screen = true;
  bool singleStep = false;
  
  Screen screen;

  if (argc < 2) {
    std::cerr << "USAGE: emu10vm <tape> [--looped][--no-gfx][--debug]" << std::endl;
    return 1;
  }

  /*std::cout << "Magic message: " << std::endl;
  for (Word w : {34, 22, 10, 28, 18, 36, 22, 18, 16, 17, 29, 36, 11, 14, 36, 11, 14, 29, 29, 14, 27, 36, 29, 17, 10, 23, 36, 33, 38, 22, 10, 28, 36, 21, 24, 21, 24, 21, 24, 21})
    std::cout << CHARSET[w];
  std::cout << std::endl;*/

  // Too lazy to use getlong_opt for now
  for (int i = 2; i < argc; i++) {
    if (std::string("--no-gfx") == argv[i]) {
      enable_screen = false;
    } else if (std::string("--looped") == argv[i]) {
      enable_looped = true;
    } else if (std::string("--debug") == argv[i]) {
      singleStep = true;
    } else {
      std::cerr << "Invalid argument: " << argv[i] << std::endl;
      return 1;
    }
  }
  
  tapeFile.open(argv[1], std::ios_base::binary);
  if (!tapeFile) {
    std::cerr << "Unable to open tape: " << argv[1] << std::endl;
    return 1;
  }
  
  Tape tape(tapeFile, enable_looped);
  std::cout << "Read " << tape.getLength() << " instructions." << std::endl;
  VM vm(tape);
  
  tapeFile.close();
  
  if (enable_screen) {
    screen.init();
    vm.screen = &screen;
  }

  
  std::set<std::size_t> breakpoints;
  
  constexpr unsigned long STEPS_PER_FRAME = 1000;
  
  while (!enable_screen || !screen.closed)
  {
    for (unsigned long i = 0; i < STEPS_PER_FRAME; ++i) {
      if (breakpoints.find(tape.getPos()) != breakpoints.end())
        singleStep = false;
      
      if (singleStep)
      {
        printf("%05zu ", tape.getPos());
        std::cout << disassemble(tape.read(), vm.conditionFlag) << std::endl;
        std::string action;
        std::getline(std::cin, action);
        //if (action.empty() || action == "s")
        //  continue;
        if (action == "r") // run
          singleStep = false;
        if (action == "d") // debug
          vm.dump(std::cout);
        if (action == "q")
          return -1;
        if (action[0] == 'b')
          breakpoints.insert(std::stoull(action.substr(2, std::string::npos)));
        if (action[0] == 'c')
          breakpoints.erase(std::stoull(action.substr(2, std::string::npos)));
      }
      
      vm.tick();
      
      if (vm.halted)
        return 1;
    }

    if (enable_screen) {
      screen.tick();
    }
  }
  
  return 0;
}

