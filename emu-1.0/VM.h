
#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <iostream>

#include <unistd.h>

#include "Tape.h"
#include "SixBit.h"
#include "Screen.h"

#include <chrono>
#include <ctime> 

class VM {
public:
  constexpr static uint32_t MEMORYSIZE = 64;

  Tape& tape;
  std::array<Word, MEMORYSIZE> memory = {};
  Screen *screen = nullptr;
  bool halted = false;
  
  bool conditionFlag = false;


  std::chrono::time_point<std::chrono::system_clock> lastClk
    = std::chrono::system_clock::now();
  
  std::string lastInput = "";

  constexpr static uint32_t RAM_SIZE = 64 * 64 * 64;
  std::array<Word, RAM_SIZE> ram = {};
  std::size_t ramAddress = 0;
  
  Word gpuX = 0;
  Word gpuY = 0;
  
  VM(Tape& tape_)
    : tape(tape_)
  {
  }
  

  int getCondition(Word opc)
  {
    return (opc - 1) / 21;
  }

  void tick() {
    Instruction instruction = tape.read();
    tape.advance();
    
    auto [OPC, A, B, C] = instruction;
    if (OPC == 0)
      return hcf();
      
    Word condition = getCondition(OPC);
    switch (condition)
    {
      case 0: break;
      case 1: if (!conditionFlag) return; break;
      case 2: if (conditionFlag) return; break;
    }
    
    Word op = OPC - 1 - 21 * condition;

    // In order of appearance in manual:
    switch (op) {
    
    case /*decimal*/ 20: return hcf();

    // (octal from here on)
    case 000: return add(A, B, C);
    case 001: return addi(A, B, C);
    case 002: return sub(A, B, C);
    case 004: return _or(A, B, C);
    case 005: return ori(A, B, C);
    case 006: return _xor(A, B, C);
    case 007: return xori(A, B, C);
    case 010: return _and(A, B, C);
    case 011: return andi(A, B, C);
    case 013: return shl(A, B, C);
    case 014: return shr(A, B, C);
    
    case 003: return comp(A, B, C);
    
    case 012: return shrti(A, B, C);
    
    case 015: return ld(A, B, C);
    case 016: return st(A, B, C);
    
    case 023: return fmu(A, B, C);
    
    case 017: return /*label*/;
    case 020: return jupdn(A, B, C, Tape::Direction::UP);
    case 021: return jupdn(A, B, C, Tape::Direction::DOWN);
    
    case 022: return io(A, B, C);
    
    default: assert(false);
    }
  }

  
  Word read(Word address)
  {
    address = address % 64;
    if (address == 0)
      return 0;
    return memory[address] % 64;
  }
  
  void write(Word address, Word value)
  {
    //if (address >= 64)
    //  std::cerr << "Address " << (int)address << " not in range!" << std::endl;
    //if (value >= 64)
    //  std::cerr << "Value " << (int)value << " not in range!" << std::endl;
    address = address % 64;
    if (address != 0)
      memory[address] = value % 64;
  }
  

  void hcf()
  {
    halted = true;
    throw std::runtime_error("Halted, caught fire");
  }

  void add(Word rd, Word ra, Word rb)
  {
    write(rd, read(ra) + read(rb));
  }
  void addi(Word rd, Word ra, Word ib)
  {
    write(rd, read(ra) + ib);
  }
  void sub(Word rd, Word ra, Word rb)
  {
    write(rd, read(ra) - read(rb));
  }
  void _or(Word rd, Word ra, Word rb)
  {
    write(rd, read(ra) | read(rb));
  }
  void ori(Word rd, Word ra, Word ib)
  {
    write(rd, read(ra) | ib);
  }
  void _xor(Word rd, Word ra, Word rb)
  {
    write(rd, read(ra) ^ read(rb));
  }
  void xori(Word rd, Word ra, Word ib)
  {
    write(rd, read(ra) ^ ib);
  }
  void _and(Word rd, Word ra, Word rb)
  {
    write(rd, read(ra) & read(rb));
  }
  void andi(Word rd, Word ra, Word ib)
  {
    write(rd, read(ra) & ib);
  }
  void shl(Word rd, Word ra, Word rb)
  {
    write(rd, read(ra) << read(rb));
  }
  void shr(Word rd, Word ra, Word rb)
  {
    write(rd, read(ra) >> read(rb));
  }

  void comp(Word aacc, Word ria, Word rib)
  {
    Word cc = aacc % 010;
    Word aa = aacc / 010;
    
    // TODO
    (void)cc;
    
//    Word lhs, rhs;
    switch (aa)
    {
    case 0: return compCM(cc, read(ria), read(rib));
    case 1: assert(false); return /*(std::cerr << "UNDOCUMENTED" << std::endl),*/
                   compCM(cc, read(rib), read(ria));
    case 2: return compCM(cc, read(ria), rib);
    case 3: return compCM(cc, ria, read(rib));
    default: assert(false);
    }
    
  }
  void compCM(Word cc, Word lhs, Word rhs)
  {
    switch (cc)
    {
    case 0: conditionFlag = true; break;
    case 1: conditionFlag = false; break;
    case 2: conditionFlag = (lhs == rhs); break;
    case 3: conditionFlag = (lhs != rhs); break;
    case 4: conditionFlag = (toSigned(lhs) < toSigned(rhs)); break;
    case 5: conditionFlag = (toSigned(lhs) > toSigned(rhs)); break;
    case 6: conditionFlag = (lhs < rhs); break;
    case 7: conditionFlag = (lhs > rhs); break;
    default: assert(false);
    }
  }
  
  void shrti(Word rd, Word ra, Word ccib)
  {
    Word cc = ccib / 010;
    Word ib = ccib % 010;
    
    switch (cc)
    {
    case 0: return shli(rd, ra, ib);
    case 1: return shri(rd, ra, ib);
    case 2: return sari(rd, ra, ib);
    case 3: return roli(rd, ra, ib);
    default: assert(false);
    }
  }
  void shli(Word rd, Word ra, Word ib)
  {
    write(rd, read(ra) << ib);
  }
  void shri(Word rd, Word ra, Word ib)
  {
    write(rd, read(ra) >> ib);
  }
  void sari(Word rd, Word ra, Word ib)
  {
    // Technically implementation defined:
    static_assert((-1 >> 4) == -1, "sar");
    std::int8_t sra = toSigned(read(ra));
    write(rd, Word(sra >> ib));
    //std::cerr << "sar " << (int)sra << " << " << (int)ib << " = " << (int)(sra >> ib) << std::endl;
  }
  void roli(Word rd, Word ra, Word ib)
  {
    write(rd, rotateLeft(ra, ib));
    //std::cerr << "rol " << (int)ra << " rot " << (int)ib << " = " << (int)rotateLeft(ra, ib) << std::endl;
  }
  static constexpr Word rotateLeft(Word value, Word bits)
  {
    assert(0 <= bits && bits < 8);
    if (bits > 6)
      bits -= 6;
    return ((value << bits) | (value >> (6-bits))) % 64;
  }
  
  void ld(Word rd, Word ra, Word ib)
  {
    write(rd, read(read(ra) + ib));
  }
  void st(Word rs, Word ra, Word ib)
  {
    write(read(ra) + ib, read(rs));
  }

  void fmu(Word rd, Word ra, Word prcc)
  {
    Word pr = prcc % 020;
    Word cc = prcc / 020;
    if (cc == 0)
    {
      //std::cerr << "fmu " << (int)read(rd) << " * " << (int)read(ra);
      unsigned tmp = unsigned(read(rd)) * unsigned(read(ra));
      tmp = tmp & 0b111111111111;
      //std::cerr << " (" << (int)tmp << ") ";
      write(rd, Word(tmp >> pr));
      //std::cerr << " >> " << (int)pr << " = " << (int)read(rd) << std::endl;
    }
    else if (cc == 1)
    {
      //std::cerr << "fms " << (int)toSigned(read(rd)) << " * " << (int)toSigned(read(ra));
      signed tmp = toSigned(read(rd)) * toSigned(read(ra));
      tmp = toSigned12(unsigned(tmp) & 0b111111111111);
      //std::cerr << " (" << (int)tmp << ") ";
      write(rd, Word(tmp >> pr));
      //std::cerr << " >> " << (int)pr << " = " << (int)read(rd) << std::endl;
    }
    else assert(false);
  }
  
  void jupdn(Word la, Word lb, Word rc, Tape::Direction dir)
  {
    // Current tape position is one below the current jump -> Undo.
    tape.advance(Tape::Direction::UP);
    
    Word lc = read(rc);
    
#ifndef NDEBUG
    std::size_t tapePos = tape.getPos();
#endif
    
    while (true)
    {
      // Different order here, but it makes sense?
      tape.advance(dir);

#ifndef NDEBUG
      if (tapePos == tape.getPos())
        assert(false); // did not find label!
#endif
      
      auto [OPC, A, B, C] = tape.read();
      if (OPC == 0)
        continue; // This does not catch fire I guess?
        
      Word condition = getCondition(OPC);
      switch (condition)
      {
        case 0: break;
        case 1: if (!conditionFlag) continue; break;
        case 2: if (conditionFlag) continue; break;
        default: assert(false);
      }
      
      Word op = OPC - 1 - 21 * condition;
      
      if (op != 017)
        continue;
      
      #if false
      printf("%05d ", tape.getPos());
      std::cout << disassemble(tape.read()) << "    ";
      std::cout << "lbl " << (int)A << " " << (int)B << " " << (int)C << " " << 64 * A + B;
      std::cout<< " - jmp " << (int)la << " " << (int)lb << " " << (int)lc << " " << 64 * la + lb << std::endl;
      #endif

      if (A != la)
        continue;
      if (B != lb)
        continue;
      if (C != lc)
        continue;
      
      // Found matching label!
      // Advance below it (no point in executing it)...
      //tape.advance(Tape::Direction::DOWN);
      return;
    }
  }
  
  void io(Word rd, Word ix, Word rs)
  {
    switch (ix)
    {
    case SERIAL_INCOMING:
      std::cout << "SERIAL_INCOMING";
      std::cout << "lastInput = " << lastInput << " sized " << lastInput.size() << std::endl;
      //if (lastInput.empty())
        std::getline(std::cin, lastInput);
      std::cout << "lastInput = " << lastInput << " sized " << lastInput.size() << std::endl;
      write(rd, std::min(63ul, lastInput.size()));
      
      usleep(5000);
      break;
    case SERIAL_READ:
      std::cout << "SERIAL_READ" << std::endl;
      while (true)
      {
        if (lastInput.empty()) {
          write(rd, -1);
          return;
        }
        
        char c = lastInput[0];
        lastInput = lastInput.substr(1, std::string::npos);
        for (Word i = 0; i < 64; ++i)
          if (CHARSET[i] == c) {
            write(rd, i);
            return;
          }
        std::cerr << "\n! invalid char !" << c << std::endl;
      }
      break;
    case SERIAL_WRITE:
      //std::cerr << "SERIAL_WRITE: " << (int)read(rs) << " " << CHARSET[read(rs)] << std::endl;
      std::cerr << CHARSET[read(rs)];
      break;
    
    case CLOCK_LO_CS:
      write(rd, getCentiseconds() & 0b111111);
      //std::cout << "CLOCK_LO_CS " << (int)read(rd) << std::endl;
      if (read(rs) != 0) {
        lastClk = std::chrono::system_clock::now();
        if (screen)
          screen->tick();
      }
      break;
    case CLOCK_HI_CS:
      write(rd, (getCentiseconds() >> 6) & 0b111111);
      //std::cout << "CLOCK_HI_CS " << (int)read(rd) << std::endl;
      if (read(rs) != 0) {
        lastClk = std::chrono::system_clock::now();
        if (screen)
          screen->tick();
      }
      break;
    
    case MEM_ADDR_HI:
      ramAddress = (ramAddress & 0b000000111111111111) | (std::size_t(read(rs)) << 12);
      break;
    case MEM_ADDR_MID:
      ramAddress = (ramAddress & 0b111111000000111111) | (std::size_t(read(rs)) << 6);
      break;
    case MEM_ADDR_LO:
      ramAddress = (ramAddress & 0b111111111111000000) | (std::size_t(read(rs)) << 0);
      break;
    case MEM_READ:
      write(rd, ram[ramAddress]);
      ramAddress = (ramAddress + 1) % RAM_SIZE;
      break;
    case MEM_WRITE:
      ram[ramAddress] = read(rs);
      ramAddress = (ramAddress + 1) % RAM_SIZE;
      break;
    
    case GPU_X:
      gpuX = read(rs);
      break;
    case GPU_Y:
      gpuY = read(rs);
      break;
    case GPU_DRAW:
      if (screen) {
        Word rgb = read(rs);
        Word r = (rgb >> 4) & 0b11;
        Word g = (rgb >> 2) & 0b11;
        Word b = (rgb >> 0) & 0b11;
        screen->draw(gpuX, gpuY, r * 0b1010101, g * 0b1010101, b * 0b1010101);
      }
      break;
      
    case DPAD:
      {
        char c;
        std::cin >> c;
        switch (c)
        {
        case 'x': return write(rd, 0b000001);
        case 'y': return write(rd, 0b000010);
        case 'w': return write(rd, 0b000100);
        case 'a': return write(rd, 0b010000);
        case 's': return write(rd, 0b001000);
        case 'd': return write(rd, 0b100000);
        }
        Word dpad = getCentiseconds() & 0b111111;
        std::cerr << "DPAD: " << std::bitset<6>(dpad) << std::endl;
        write(rd, dpad);
        break;
      }
    
    default:
      throw std::runtime_error("IO DEVICE NOT CONNECTED");
    }
  }
  
  std::size_t getCentiseconds()
  {
    auto timeDiff = std::chrono::system_clock::now() - lastClk;
    std::size_t centiseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count() / 10;
    return std::min(centiseconds, 64ul * 64ul - 1);
  }
  
  void dump(std::ostream& s)
  {
    s << "============================" << std::endl;
    for (Word i = 0; i < MEMORYSIZE; ++i)
    {
      Word ri = read(i);
      if (ri == 0)
        continue;
      std::bitset<6> bits(ri);
      s << "  r" << two(i) << ": " << bits << "  (" << (int)ri << ")" << std::endl;
    }
    s << "Condition flag: " << conditionFlag << std::endl;
    s << "============================" << std::endl;
  }
  
  
  static constexpr Word SERIAL_INCOMING = 0;
  static constexpr Word SERIAL_READ = 1;
  static constexpr Word SERIAL_WRITE = 2;
  
  static constexpr Word CLOCK_LO_CS = 3;
  static constexpr Word CLOCK_HI_CS = 4;
  
  static constexpr Word MEM_ADDR_HI = 020;
  static constexpr Word MEM_ADDR_MID = 021;
  static constexpr Word MEM_ADDR_LO = 022;
  static constexpr Word MEM_READ = 023;
  static constexpr Word MEM_WRITE = 024;
  
  static constexpr Word GPU_X = 025;
  static constexpr Word GPU_Y = 026;
  static constexpr Word GPU_DRAW = 027;
  
  static constexpr Word DPAD = 030;
};


void tests()
{
  static_assert(VM::rotateLeft(0xF, 0) == 0xF, "rol");
  static_assert(VM::rotateLeft(0xF, 6) == 0xF, "rol");
  static_assert(VM::rotateLeft(0x1, 1) == 0x2, "rol");
  static_assert(VM::rotateLeft(0b100110, 2) == 0b011010, "rol");
  static_assert(VM::rotateLeft(0b100110, 6-2) == 0b101001, "rol");
}


