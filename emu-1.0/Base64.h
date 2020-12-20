// Lookup table from https://stackoverflow.com/a/34571089
#pragma once

#include <cassert>
#include <vector>
#include <string_view>

#include "SixBit.h"

constexpr std::string_view BASE64_CHARS =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline Word decode(char in)
{
  static std::vector<Word> LOOKUP = []() {
        std::vector<Word> T(256, -1);
        for (unsigned i=0; i<64; i++)
          T[BASE64_CHARS[i]] = i;
        return T;
    }();
  return LOOKUP[(unsigned char)in];
}

inline char encode(Word in)
{
  return BASE64_CHARS[in];
}
