
#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cctype>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

#include "Base64.h"
#include "SixBit.h"


void triml(std::string_view& sv)
{
  while (sv.size() > 0 && std::isspace(sv.front()))
    sv.remove_prefix(1);
}

void trimr(std::string_view& sv)
{
  while (sv.size() > 0 && std::isspace(sv.back()))
    sv.remove_suffix(1);
}

void trim(std::string_view& sv)
{
  triml(sv);
  trimr(sv);
}

void checkValid(std::string_view sv, std::string_view line)
{
  if (sv.empty())
    throw std::runtime_error(std::string("Invalid asm: ").append(line));
}

bool startsWith(std::string_view sv, std::string_view seq)
{
  if (sv.size() < seq.size())
    return false;
  return (sv.substr(0, seq.size()) == seq);
}

std::string_view popToken(std::string_view& sv, bool commaAllowed = true, char del = ' ')
{
  auto spacePos = sv.find(del);
  // Must have multiple tokens left.
  assert(spacePos != std::string_view::npos);
  auto ret = sv.substr(0, spacePos);
  // Trim down input.
  sv.remove_prefix(spacePos+1);
  triml(sv);
  
  assert(!ret.empty());
  if (commaAllowed && ret.back() == ',')
    ret.remove_suffix(1);
  return ret;
}

bool hasMultipleTokens(std::string_view& sv, char del = ' ')
{
  auto delPos = sv.find(del);
  if (delPos == 0)
    return false;
  if (delPos == sv.size() - 1)
    return false;
  if (delPos == std::string_view::npos)
    return false;
  return true;
}

class Assembler
{
public:
  Assembler(std::istream& input, std::ostream& output)
    : _in(input)
    , _out(output)
  {
  }
  
  ~Assembler()
  {
  }
  
  void check(bool condition, std::string msg)
  {
    if (!condition)
    {
      std::cerr << "Error on line: " << _lastLine << std::endl;
      std::cerr << msg << std::endl;
      exit(1);
    }
  }
  
  int toInt2(std::string_view sv, int maxVal)
  {
    assert(sv.size() == 2);
    std::size_t pos;
    int r = std::stoi(std::string(sv), &pos);
    check(pos == 2 && r >= 0 && r < maxVal, "invalid numeric part");
    return r;
  }
  
  Word parseReg(std::string_view sv)
  {
    check(sv.size() == 3 && sv[0] == 'r', "invalid register");
    sv.remove_prefix(1);
    return (Word)toInt2(sv, 64);
  }
  
  int parseInt(std::string_view sv, int upperBound = 64)
  {
    check(!sv.empty(), "invalid immediate");
    std::size_t pos;
    int i = std::stoi(std::string(sv), &pos, 0);
    check(pos == sv.size(), "invalid immediate");
    check(i >= 0 && i < upperBound, "invalid immediate");
    return i;
  }
  
  Word parseImm(std::string_view sv, int upperBound = 64)
  {
    return (Word)parseInt(sv, upperBound);
  }
  
  bool isReg(std::string_view sv)
  {
    return sv[0] == 'r';
  }
  
  void emit(Word OPC, Word A, Word B, Word C)
  {
    _out << encode(OPC) << encode(A) << encode(B) << encode(C);
  }
  
  void assemble()
  {
    while (std::getline(_in, _lastLine))
    {
      Word condition = 0;
      // Strip comment
      std::string_view current =
        std::string_view(_lastLine).substr(0, _lastLine.find("#", 0));
      
      while (!current.empty() && std::isdigit(current.front()))
        current.remove_prefix(1);
          
      trim(current);
      if (current.empty())
        continue;
      
      if (current[0] == '+') {
        current.remove_prefix(1);
        condition = 1;
      }
      else if (current[0] == '-') {
        current.remove_prefix(1);
        condition = 2;
      }
      
      trim(current);
      check(!current.empty(), "syntax error");
      
      Word op = 0;
      Word A = 0;
      Word B = 0;
      Word C = 0;
      
      if (startsWith(current, "hcf")) {
        assert(current == "hcf");
        assert(condition == 0);
        emit(0, 0, 0, 0);
        continue;
      }
      
      if (startsWith(current, "add"))
        getAdd(current, op, A, B, C);
      else if (startsWith(current, "sub"))
        getSub(current, op, A, B, C);
      else if (startsWith(current, "or"))
        getOr(current, op, A, B, C);
      else if (startsWith(current, "xor"))
        getXor(current, op, A, B, C);
      else if (startsWith(current, "and"))
        getAnd(current, op, A, B, C);
      else if (startsWith(current, "shl"))
        getShl(current, op, A, B, C);
      else if (startsWith(current, "shr"))
        getShr(current, op, A, B, C);
      else if (startsWith(current, "cmp"))
        getCmp(current, op, A, B, C);
      else if (startsWith(current, "sar") || startsWith(current, "rol"))
        getSarRol(current, op, A, B, C);
      else if (startsWith(current, "ld"))
        getLd(current, op, A, B, C);
      else if (startsWith(current, "st"))
        getSt(current, op, A, B, C);
      else if (startsWith(current, "fmu") || startsWith(current, "fms"))
        getFmpr(current, op, A, B, C);
      else if (startsWith(current, "lbl"))
        getLbl(current, op, A, B, C);
      else if (startsWith(current, "jup") || startsWith(current, "jdn"))
        getJupdn(current, op, A, B, C);
      else if (startsWith(current, "io"))
        getIo(current, op, A, B, C);
      // extension
      else if (startsWith(current, "mov"))
        getMov(current, op, A, B, C);
      else
        check(false, "unknown opcode: '" + std::string(current) + "'");
      
      Word OPC = 21 * condition + op + 1;
      emit(OPC, A, B, C);
    }
    
  }
  
protected:
  std::array<std::string_view, 4> pop4(std::string_view& current)
  {
    check(hasMultipleTokens(current), "too few arguments");
    auto t0 = popToken(current, false);
    check(hasMultipleTokens(current), "too few arguments");
    auto t1 = popToken(current);
    check(hasMultipleTokens(current), "too few arguments");
    auto t2 = popToken(current);
    check(!hasMultipleTokens(current), "too many arguments");
    auto t3 = current;
    return {t0, t1, t2, t3};
  }
  
  std::array<std::string_view, 3> pop3(std::string_view& current)
  {
    check(hasMultipleTokens(current), "too few arguments");
    auto t0 = popToken(current, false);
    check(hasMultipleTokens(current), "too few arguments");
    auto t1 = popToken(current);
    check(!hasMultipleTokens(current), "too many arguments");
    auto t2 = current;
    return {t0, t1, t2};
  }
  
  void getAdd(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, rib] = pop4(current);
    check(opstr == "add" || opstr == "addi", "unknown opcode");
    A = parseReg(rd);
    B = parseReg(ra);
    if (isReg(rib)) {
      // only add allowed
      check(opstr != "addi", "immediate required");
      op = 000;
      C = parseReg(rib);
    } else {
      // add or addi allowed
      op = 001;
      C = parseImm(rib);
    }
  }
  
  void getSub(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, rb] = pop4(current);
    check(opstr == "sub", "unknown opcode");
    check(isReg(rb), "register required");
    A = parseReg(rd);
    B = parseReg(ra);
    op = 002;
    C = parseReg(rb);
  }
  
  void getOr(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, rib] = pop4(current);
    check(opstr == "or" || opstr == "ori", "unknown opcode");
    A = parseReg(rd);
    B = parseReg(ra);
    if (isReg(rib)) {
      // only or allowed
      check(opstr != "ori", "immediate required");
      op = 004;
      C = parseReg(rib);
    } else {
      // or or ori allowed
      op = 005;
      C = parseImm(rib);
    }
  }
  
  void getXor(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, rib] = pop4(current);
    check(opstr == "xor" || opstr == "xori", "unknown opcode");
    A = parseReg(rd);
    B = parseReg(ra);
    if (isReg(rib)) {
      // only xor allowed
      check(opstr != "xori", "immediate required");
      op = 006;
      C = parseReg(rib);
    } else {
      // xor or xori allowed
      op = 007;
      C = parseImm(rib);
    }
  }
  
  void getAnd(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, rib] = pop4(current);
    check(opstr == "and" || opstr == "andi", "unknown opcode");
    A = parseReg(rd);
    B = parseReg(ra);
    if (isReg(rib)) {
      // only and allowed
      check(opstr != "andi", "immediate required");
      op = 010;
      C = parseReg(rib);
    } else {
      // and or andi allowed
      op = 011;
      C = parseImm(rib);
    }
  }
  
  void getShl(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, rib] = pop4(current);
    check(opstr == "shl" || opstr == "shli", "unknown opcode");
    A = parseReg(rd);
    B = parseReg(ra);
    if (isReg(rib)) {
      // only shl allowed
      check(opstr != "shli", "immediate required");
      op = 013;
      C = parseReg(rib);
    } else {
      // shl or shli allowed
      op = 012;
      Word imm = parseImm(rib, 8);
      C = 000 + imm;
    }
  }
  
  void getShr(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, rib] = pop4(current);
    check(opstr == "shr" || opstr == "shri", "unknown opcode");
    A = parseReg(rd);
    B = parseReg(ra);
    if (isReg(rib)) {
      // only shr allowed
      check(opstr != "shri", "immediate required");
      op = 014;
      C = parseReg(rib);
    } else {
      // shr or shri allowed
      op = 012;
      Word imm = parseImm(rib, 8);
      C = 010 + imm;
    }
  }
  
  void getCmp(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, ra, rb] = pop3(current);
    check(opstr.size() == 5, "unknown opcode");
    std::array<std::string_view, 8> CM = { "tr", "fa", "eq", "ne", "sl", "sg", "ul", "ug" };
    auto cc = std::find(CM.begin(), CM.end(), opstr.substr(3, 2)) - CM.begin();
    check(cc < 8, "unknown opcode");
    op = 003;
    if (isReg(ra) && isReg(rb)) {
      A = 000 + cc;
      B = parseReg(ra);
      C = parseReg(rb);
    } else if (isReg(ra)) {
      A = 020 + cc;
      B = parseReg(ra);
      C = parseImm(rb);
    } else {
      check(isReg(rb), "need at least one register");
      A = 030 + cc;
      B = parseImm(ra);
      C = parseReg(rb);
    }
  }
  
  void getSarRol(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra, ib] = pop4(current);
    check(opstr == "sar" || opstr == "rol", "unknown opcode");
    A = parseReg(rd);
    B = parseReg(ra);
    Word imm = parseImm(ib, 8);
    op = 12;
    if (opstr == "sar")
      C = 020 + imm;
    else
      C = 030 + imm;
  }
  
  void getLd(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    check(hasMultipleTokens(current), "too few arguments");
    auto opstr = popToken(current, false);
    check(opstr == "ld", "unknown opcode");
    op = 015;
    
    check(hasMultipleTokens(current), "too few arguments");
    auto rd = popToken(current);
    A = parseReg(rd);
    
    check(current.size() > 2, "invalid argument");
    check(current.front() == '[' && current.back() == ']', "invalid argument");
    current.remove_prefix(1);
    current.remove_suffix(1);
    trim(current);
    if (hasMultipleTokens(current, '+')) {
      auto ra = popToken(current, false, '+');
      auto ib = current;
      trim(ra);
      trim(ib);
      B = parseReg(ra);
      C = parseImm(ib);
    } else {
      auto rib = current;
      if (isReg(rib)) {
        B = parseReg(rib);
        C = 0;
      } else {
        B = 0;
        C = parseImm(rib);
      }
    }
  }
  
  void getSt(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    check(hasMultipleTokens(current), "too few arguments");
    auto opstr = popToken(current, false);
    check(opstr == "st", "unknown opcode");
    op = 016;
    
    check(hasMultipleTokens(current), "too few arguments");
    check(current.front() == '[', "invalid argument");
    auto endPos = current.find(']');
    check(endPos != std::string_view::npos, "invalid argument");
    auto raib = current.substr(1, endPos-1);
    current.remove_prefix(endPos+1);
    check(current.size() > 3, "invalid argument");
    if (current.front() == ',')
      current.remove_prefix(1);
    triml(current);
    auto rs = current;
    A = parseReg(rs);
    
    if (hasMultipleTokens(raib, '+')) {
      auto ra = popToken(raib, false, '+');
      auto ib = raib;
      trim(ra);
      trim(ib);
      B = parseReg(ra);
      C = parseImm(ib);
    } else {
      auto rib = raib;
      if (isReg(rib)) {
        B = parseReg(rib);
        C = 0;
      } else {
        B = 0;
        C = parseImm(rib);
      }
    }
  }
  
  void getFmpr(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ra] = pop3(current);
    check(opstr.size() == 5 || opstr.size() == 4, "unknown opcode");
    auto pr = opstr.substr(3, 2);
    Word cc = (Word)toInt2(pr, 16);
    op = 023;
    A = parseReg(rd);
    B = parseReg(ra);
    if (opstr.substr(0, 3) == "fmu")
      C = 000 + cc;
    else
      C = 020 + cc;
  }
  
  void getLbl(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    check(hasMultipleTokens(current), "too few arguments");
    auto opstr = popToken(current, false);
    check(opstr == "lbl", "unknown opcode");
    std::string_view lab, lc;
    if (hasMultipleTokens(current)) {
      lab = popToken(current);
      lc = current;
    } else {
      lab = current;
      lc = "0";
    }
    int labelValue = parseInt(lab, 4096);
    op = 017;
    A = labelValue / 64;
    B = labelValue % 64;
    C = parseImm(lc);
  }
  
  void getJupdn(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    check(hasMultipleTokens(current), "too few arguments");
    auto opstr = popToken(current, false);
    check(opstr == "jup" || opstr == "jdn", "unknown opcode");
    std::string_view lab, rc;
    if (hasMultipleTokens(current)) {
      lab = popToken(current);
      rc = current;
    } else {
      lab = current;
      rc = "r00";
    }
    int labelValue = parseInt(lab, 4096);
    op = (opstr == "jup" ? 020 : 021);
    A = labelValue / 64;
    B = labelValue % 64;
    C = parseReg(rc);
  }
  
  void getIo(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    check(hasMultipleTokens(current), "too few arguments");
    auto opstr = popToken(current, false);
    check(opstr == "io", "unknown opcode");
    std::string_view rd, ix, rs;
    if (!hasMultipleTokens(current)) {
      rd = "r00";
      ix = current;
      rs = "r00";
    } else {
      std::string_view rdix = popToken(current);
      if (!isReg(rdix)) {
        rd = "r00";
        ix = rdix;
        rs = current;
      } else {
        rd = rdix;
        if (hasMultipleTokens(current)) {
          ix = popToken(current);
          rs = current;
        } else {
          ix = current;
          rs = "r00";
        }
      }
    }
    op = 022;
    A = parseReg(rd);
    B = parseDeviceImm(ix);
    C = parseReg(rs);
  }
  
  void getMov(std::string_view current, Word& op, Word& A, Word& B, Word& C)
  {
    auto [opstr, rd, ria] = pop3(current);
    check(opstr == "mov", "unknown opcode, expected mov");
    // This is just an addi in disguise.
    op = 001;
    A = parseReg(rd);
    if (isReg(ria)) {
      B = parseReg(ria);
      C = 0;
    } else {
      B = 0;
      C = parseImm(ria);
    }
  }
  
  Word parseDeviceImm(std::string_view sv)
  {
    static const std::map<std::string_view, Word> CONSTANTS = {
      { "SERIAL_INCOMING", 0 },
      { "SERIAL_READ", 1 },
      { "SERIAL_WRITE", 2 },
      { "CLOCK_LO_CS", 3 },
      { "CLOCK_HI_CS", 4 },
      { "MEM_ADDR_HI", 020 },
      { "MEM_ADDR_MID", 021 },
      { "MEM_ADDR_LO", 022 },
      { "MEM_READ", 023 },
      { "MEM_WRITE", 024 },
      { "GPU_X", 025 },
      { "GPU_Y", 026 },
      { "GPU_DRAW", 027 },
      { "DPAD", 030 },
    };
    auto it = CONSTANTS.find(sv);
    if (it != CONSTANTS.end())
      return it->second;
    return parseImm(sv);
  }
  
private:
  std::istream& _in;
  std::ostream& _out;
  std::string _lastLine;
};


