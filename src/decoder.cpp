#include "decoder.hpp"

#include "exception.hpp"

constexpr Decoder::Decoder(Uxlen instruction) {
  Opcode opcode{static_cast<Opcode>((instruction >> 2) & 0b11111)};

  if ((instruction & 0b11) != 0b11) throw Errors::Illegal_instruction{""};
}
