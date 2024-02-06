#include "decoder.hpp"

#include "exception.hpp"

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
    return (data >> start_pos) & static_cast<T>(make_mask<unsigned long long>(end_pos-start_pos+1));
  }

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, std::size_t pos) {
    return extract_bits(data, pos, pos);
  }
}

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
