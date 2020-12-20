
#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <string>

// 6 bit value
/*struct Word
{
  Word(std::uint8_t v) : value(std::min(v, 63) {}
  
  // implicit conversion... fite me
  operator std::uint8_t() { return value; }
  
  std::uint8_t value;
};*/

using Word = std::uint8_t;

// Performs sign extension from 6 bits to 8 bits.
constexpr std::int8_t toSigned(Word word)
{
  assert(word < 64u);
  std::int8_t s(word); // this is purely for signed type - sign is still positive!
  assert(s >= 0);
  if (word < 32u)
    return s;
  return 0 - (64 - s);
}

constexpr signed toSigned12(unsigned doubleWord)
{
  assert(doubleWord < 64u * 64u);
  signed s(doubleWord); // this is purely for signed type - sign is still positive!
  assert(s >= 0);
  if (doubleWord < 64u * 64u / 2u)
    return s;
  return 0 - (64u * 64u - s);
}


std::bitset<6> toBits(Word word)
{
  std::bitset<6> b(word);
  return b;
}

static_assert(toSigned(0b100000) == -32);
static_assert(toSigned(0b100001) == -31);
static_assert(toSigned(0b111111) == -1);
static_assert(toSigned(0b000000) == 0);
static_assert(toSigned(0b000001) == 1);
static_assert(toSigned(0b011111) == 31);

using Instruction = std::array<Word, 4>;

void check(Instruction instruction)
{
  assert(instruction[0] < 64);
  assert(instruction[1] < 64);
  assert(instruction[2] < 64);
  assert(instruction[3] < 64);
}

const std::string CHARSET = {
  "0123456789ABCDEF"
  "GHIJKLMNOPQRSTUV"
  "WXYZ +-*/<=>()[]"
  "{}#$_?|^&!~,.:\n§" // using § for invalid char
};

