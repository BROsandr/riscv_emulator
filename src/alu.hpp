#pragma once

#include "riscv.hpp"

class Alu {
  public:
    enum Op {
      ADD  = 0b00000,
      SUB  = 0b01000,
      XOR  = 0b00100,
      OR   = 0b00110,
      AND  = 0b00111,
      SRA  = 0b01101,
      SRL  = 0b00101,
      SLL  = 0b00001,
      LTS  = 0b11100,
      LTU  = 0b11110,
      GES  = 0b11101,
      GEU  = 0b11111,
      EQ   = 0b11000,
      NE   = 0b11001,
      SLTS = 0b00010,
      SLTU = 0b00011,
    };
    constexpr Alu(Op op, Uxlen a, Uxlen b);

    constexpr Uxlen get_result() const { return result; }
    constexpr bool  get_flag  () const { return flag;   }

  private:
    Uxlen result{};
    bool flag  {};
};
