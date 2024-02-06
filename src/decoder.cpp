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

  template <typename T>
  constexpr T sign_extend(T data, std::size_t sign_pos) {
    assert(sign_pos < (sizeof(data) * 8));

    T m{1U << (sign_pos-1)};
    return (data ^ m) - m;
  }
}

constexpr Decoder::Decoder(Uxlen instruction) : m_instruction{instruction} {
  const Opcode opcode{static_cast<Opcode>(extract_bits(instruction, 2, 6))};

  const auto funct3{extract_bits(instruction, 12, 14)};
  const auto funct7{extract_bits(instruction, 25, 31)};

  const std::size_t ra1{extract_bits(instruction, 15, 19)};
  const std::size_t ra2{extract_bits(instruction, 20, 24)};
  const std::size_t wa {extract_bits(instruction, 7, 11)};

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
      Callback::op(alu_op, ra1, ra2, wa);
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
      Callback::op_imm(alu_op, ra1, get_imm_i(), wa);
    } break;

    case Opcode::lui : Callback::op_imm(get_imm_u(), wa); break;

    case Opcode::load: {
      if (((funct3 & 0b11) == 0b11) || funct3 == 6) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::load"};
      }
      Callback::load(funct3, get_imm_i(), wa);
    } break;

    case Opcode::store: {
      if (funct3 >= 3) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::store"};
      }
      Callback::store(funct3, get_imm_i());
    } break;

    case Opcode::branch: {
      if ((funct3 == 2) || (funct3 == 3)) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::branch"};
      }
      Callback::branch((0b11 << 3) | funct3);
    } break;

    case Opcode::jal: Callback::jal(); break;

    case Opcode::jalr: {
      if (funct3 != 0) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::jalr"};
      }

      Callback::jalr();
    } break;

    case Opcode::auipc: Callback::auipc(); break;

    case Opcode::misc_mem: {
      if (funct3 != 0) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::misc_mem"};
      }
    } break;

    case Opcode::system: {
      bool mret{};

      if (funct3 == 0) {
        if (extract_bits(instruction, 7, 31) == 0b0011000000100000000000000) {
          mret = true;
        } else {
          throw Errors::Illegal_instruction{instruction, "illegal instr[31:7] for Opcode::system"};
        }
      }

      bool csr_we{funct3 != 0};
      bool gpr_we{funct3 != 0};
      Callback::system(mret, csr_we, gpr_we);
    } break;
  }
}

constexpr int Decoder::func7() const {
  return 0;
}
constexpr int Decoder::func3() const {
  return 0;
}

constexpr Uxlen Decoder::get_imm_i    () const {
  const Uxlen unextended{extract_bits(m_instruction, 20, 31)};
  return sign_extend(unextended, 12);
}
constexpr Uxlen Decoder::get_imm_u    () const {
  return extract_bits(m_instruction, 12, 31) << 12;
}
constexpr Uxlen Decoder::get_imm_s    () const {
  const Uxlen unextended{(extract_bits(m_instruction, 25, 31) << 5) | extract_bits(m_instruction, 7, 11)};
  return sign_extend(unextended, 12);
}
constexpr Uxlen Decoder::get_imm_b    () const {
  const Uxlen unextended{(extract_bits(m_instruction, 7) << (5 + 6)) |
      (extract_bits(m_instruction, 25, 30) << 5) |
      (extract_bits(m_instruction, 8, 11) << 1)};

  return sign_extend(unextended, 11);
}
constexpr Uxlen Decoder::get_imm_j    () const {
  return 0;
}
constexpr Uxlen Decoder::get_imm_zicsr() const {
  return 0;
}
