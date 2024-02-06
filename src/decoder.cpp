#include "decoder.hpp"

#include "exception.hpp"

#include <cassert>

namespace {
  template <typename T>
  constexpr T make_mask(std::size_t len, std::size_t pos = 0) {
    return ((static_cast<T>(1) << len)-1) << pos;
  }

  template<typename T>
  concept Right_shiftable = requires(T a) {
    a >> 1;
  };

  template<typename T>
  concept Andable = requires(T a) {
    a & static_cast<T>(1);
  };

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, std::size_t start_pos, std::size_t end_pos) {
    assert(end_pos >= start_pos);
    return (data >> start_pos) & static_cast<T>(make_mask<unsigned long long>(end_pos-start_pos+1));
  }

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, std::size_t pos) {
    return extract_bits(data, pos, pos);
  }
}

constexpr Decoder::Decoder(Uxlen instruction) {
  const Opcode opcode{static_cast<Opcode>(extract_bits(instruction, 2, 6))};

  const auto funct3{extract_bits(instruction, 12, 14)};
  const auto funct7{extract_bits(instruction, 25, 31)};

  if ((instruction & 0b11) != 0b11) {
    throw Errors::Illegal_instruction{instruction, "(instruction & 0b11) != 0b11"};
  }

  switch (opcode) {
    case Opcode::op: {
      if ((((extract_bits(funct7, 6) << 5) | extract_bits(funct7, 0, 4)) == 0) ||
          (((extract_bits(funct7, 5) == 1) && (funct3 != 0) && (funct3 != 5)) == 0)) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 and/or funct7 for Opcode::op"};
      }

      const Alu::Op alu_op{static_cast<Alu::Op>(
          (extract_bits(funct7, 5) << 3) | funct3
      )};
      Callback::op(alu_op);
    } break;

    case Opcode::op_imm: {
      Alu::Op alu_op{};
      if ((funct3 & 0b11) == 0b01) {
        alu_op = static_cast<Alu::Op>((extract_bits(funct7, 5) << 3) | funct3);
        if (((funct3 == 0b101) &
            (((extract_bits(funct7, 6) << 5) | extract_bits(funct7, 0, 4)) != 0)) |
            ((funct3 == 0b001) & (funct7 != 0))) {
          throw Errors::Illegal_instruction{instruction, "illegal funct3 and/or funct7 for Opcode::op_imm"};
        }
      } else {
        alu_op = static_cast<Alu::Op>(funct3);
      }
      Callback::op_imm(alu_op);
    } break;
  }
}
