
#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>

#include "SixBit.h"
#include "Base64.h"

class Tape {
public:
  enum class Direction { UP, DOWN };
  
  Tape(std::istream& f, bool looped)
    : _instructions()
    , _pos(0)
    , _looped(looped)
    , _reads(0)
  {
    std::string buf(4, ' ');
    while (!f.eof() && f.peek() != '\n')
    {
      f.read(buf.data(), 4);
      _instructions.push_back({decode(buf[0]), decode(buf[1]), decode(buf[2]), decode(buf[3])});
      check(_instructions.back());
    }
  }
  
  Instruction read()
  {
    assert(_pos < _instructions.size());
    _reads++;
    //std::cout << "Read pos " << _pos << std::endl;
    return _instructions[_pos];
  }
  
  void advance(Direction dir = Direction::DOWN)
  {
    if (dir == Direction::DOWN)
    {
      if (_looped && _pos == _instructions.size() - 1)
        _pos = 0;
      else
        ++_pos;
    }
    else // UP
    {
      if (_looped && _pos == 0)
        _pos = _instructions.size() - 1;
      else
        --_pos;
    }
    // Ok, that could be done with less code.
  }
  
  std::size_t getPos()
  { return _pos; }
  
  std::size_t eof()
  {
    return (_pos == (std::size_t)-1) || (_pos == _instructions.size());
  }
  
  std::size_t getLength()
  {
    return _instructions.size();
  }
  
private:
  std::vector<Instruction> _instructions;
  std::size_t _pos;
  bool _looped;
  std::size_t _reads;
};

