
#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <sstream>

#include "SixBit.h"

std::string two(Word r)
{
  // Screw format strings!
  std::string s = std::to_string((int)r);
  if (s.size() == 1)
    s = "0" + s;
  return s;
}

std::string reg(Word r)
{
  return "r" + two(r);
}

std::string imm(Word i)
{
  return std::to_string((int)i);
}

int getCondition(Word opc)
{
  return (opc - 1) / 21;
}

std::string disassemble(Instruction instruction, int conditionFlagStatus = -1)
{
  std::stringstream ret;

  auto [OPC, A, B, C] = instruction;
  if (OPC == 0)
    return "hcf";
    
  Word condition = getCondition(OPC);
  bool disabled = false;
  if (conditionFlagStatus != -1)
  {
    if (condition == 1 && conditionFlagStatus == 0)
      disabled = true;
    if (condition == 2 && conditionFlagStatus == 1)
      disabled = true;
      
    ret << (disabled ? "(" : " ");
  }
  
  switch (condition)
  {
    case 0: ret << "  "; break;
    case 1: ret << "+ "; break;
    case 2: ret << "- "; break;
  }
  
  Word op = OPC - 1 - 21 * condition;

  // In order of appearance in manual:
  switch (op) {
  
  case /*decimal*/ 20: ret << "hcf"; break;

  // (octal from here on)
  case 000: ret << "add   " << reg(A) << ", " << reg(B) << ", " << reg(C); break;
  case 001: if (B == 0)
              ret << "mov   " << reg(A) << ", " << imm(C);
            else if (C == 0)
              ret << "mov   " << reg(A) << ", " << reg(B);
            else
              ret << "addi  " << reg(A) << ", " << reg(B) << ", " << imm(C);
            break;
  case 002: ret << "sub   " << reg(A) << ", " << reg(B) << ", " << reg(C); break;
  case 004: ret << "or    " << reg(A) << ", " << reg(B) << ", " << reg(C); break;
  case 005: ret << "ori   " << reg(A) << ", " << reg(B) << ", " << imm(C); break;
  case 006: ret << "xor   " << reg(A) << ", " << reg(B) << ", " << reg(C); break;
  case 007: ret << "xori  " << reg(A) << ", " << reg(B) << ", " << imm(C); break;
  case 010: ret << "and   " << reg(A) << ", " << reg(B) << ", " << reg(C); break;
  case 011: ret << "andi  " << reg(A) << ", " << reg(B) << ", " << imm(C); break;
  case 013: ret << "shl   " << reg(A) << ", " << reg(B) << ", " << reg(C); break;
  case 014: ret << "shr   " << reg(A) << ", " << reg(B) << ", " << reg(C); break;
  
  case 003: 
  {
    static std::array<const char*, 8> CM = { "tr", "fa", "eq", "ne", "sl", "sg", "ul", "ug" };
    ret << "cmp" << CM[A % 010] << " ";
    switch (A / 010)
    {
    case 0: ret << reg(B) << ", " << reg(C); break;
    case 1: ret << reg(C) << ", " << reg(B) << "# UNDOCUMENTED"; break;
    case 2: ret << reg(B) << ", " << imm(C); break;
    case 3: ret << imm(B) << ", " << reg(C); break;
    default: assert(false);
    }
  }
  break;
  
  case 012:
  {
    static std::array<const char*, 4> OPC = { "shli  ", "shri  ", "sari  ", "roli  " }; // await UB
    ret << OPC[C / 010] << reg(A) << ", " << reg(B) << ", " << imm(C % 010);
  }
  break;
  
  case 015: ret << "ld    " << reg(A) << ", [" << reg(B) << "+" << imm(C) << "]"; break;
  case 016: ret << "st    [" << reg(B) << "+" << imm(C) << "], " << reg(A); break;
  
  case 023: 
  {
    ret << ((C / 020 == 0) ? "fmu" : "fms") << two(C % 020) << " ";
    ret << reg(A) << ", " << reg(B);
  }
  break;
  
  case 017: ret << "lbl   " << unsigned(A) * 64 + unsigned(B);
            if (C) ret << ", " << imm(C);
            break;
  case 020: ret << "jup   " << unsigned(A) * 64 + unsigned(B);
            if (C) ret << ", " << reg(C);
            break;
  case 021: ret << "jdn   " << unsigned(A) * 64 + unsigned(B);
            if (C) ret << ", " << reg(C);
            break;
  
  case 022: ret << "io    " << reg(A) << ", " << imm(B) << ", " << reg(C); break;
  
  default: assert(false);
  }
  
  if (conditionFlagStatus != -1)
    ret << (disabled ? ")" : " ");
  
  return ret.str();
}

