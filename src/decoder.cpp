#include "decoder.hpp"

#include "exception.hpp"

constexpr Decoder::Decoder(Uxlen instruction) {
  const Opcode opcode{static_cast<Opcode>((instruction >> 2) & 0b11111)};

  const int funct3{static_cast<int>((instruction >> 12) & 0b111)};

  if ((instruction & 0b11) != 0b11) {
    throw Errors::Illegal_instruction{instruction, "(instruction & 0b11) != 0b11"};
  }

  switch (opcode) {
    case Opcode::op: {
      int alu_op{};
      op_opcode(funct7);
    }
  }
}
