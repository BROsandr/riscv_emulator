#include "decoder.hpp"

#include "exception.hpp"

#include <cassert>
#include <climits>

#include <iostream>
#include <optional>

namespace {
  class Bit_range {
    public:
      constexpr Bit_range(std::size_t msb, std::size_t lsb)
          : m_msb{msb}, m_lsb{lsb}, m_width{msb - lsb + 1} {
        assert(msb >= lsb);
      }
      explicit constexpr Bit_range(std::size_t pos)
          : Bit_range(pos, pos) {}

      constexpr std::size_t get_msb()   const {return m_msb;}
      constexpr std::size_t get_lsb()   const {return m_lsb;}
      constexpr std::size_t get_width() const {return m_width;}

      constexpr bool is_overlapping(const Bit_range& rhs) const {
        return get_lsb() <= rhs.get_msb() && rhs.get_lsb() <= get_msb();
      }

      friend std::ostream& operator<<(std::ostream& os, const Bit_range& range) {
          os << "{" << range.get_msb() << "," << range.get_lsb() << "}";
          return os;
      }

    private:
      const std::size_t m_msb;
      const std::size_t m_lsb;
      const std::size_t m_width;
  };

  constexpr bool is_overlapping(const Bit_range& lhs, const Bit_range& rhs) {
    return lhs.get_lsb() <= rhs.get_msb() && rhs.get_lsb() <= lhs.get_msb();
  }

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
  constexpr T extract_bits(T data, Bit_range range) {
    assert(range.get_msb() < (sizeof(T) * CHAR_BIT));
    return (data >> range.get_lsb()) & static_cast<T>(make_mask<unsigned long long>(range.get_width()));
  }

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, std::size_t pos) {
    return extract_bits(data, {pos, pos});
  }

  template <typename T, typename ...Q>
  constexpr T extract_bits(T data, Bit_range lsb_bit_range, Q ...msb_bit_ranges) {
    return (extract_bits(data, msb_bit_ranges...) << lsb_bit_range.get_width()) | extract_bits(data, lsb_bit_range);
  }

  template <typename T>
  constexpr T sign_extend(T data, std::size_t sign_pos) {
    assert(sign_pos < (sizeof(data) * 8));

    T m{1U << (sign_pos-1)};
    return (data ^ m) - m;
  }

  constexpr unsigned int get_funct3(Uxlen instruction) {
    return extract_bits(instruction, {14, 12});
  }

  constexpr unsigned int get_funct7(Uxlen instruction) {
    return extract_bits(instruction, {31, 25});
  }

  std::string to_string(Decoder::Isa_extension extension) {
    using enum Decoder::Isa_extension;
    switch (extension) {
      case isa_zicsr: return "Zicsr";
      default: assert((void("Unknown isa_extension" + std::to_string(extension)), 0));
    }
  }
}

constexpr Decoder::Concrete_instruction Decoder::decode_concrete_instruction(Uxlen instruction) {
  if ((instruction & 0b11) != 0b11) {
    throw Errors::Illegal_instruction{instruction, "(instruction & 0b11) != 0b11"};
  }

  const Opcode opcode{static_cast<Opcode>(extract_bits(instruction, {6, 2}))};

  // using enum Decoder::Concrete_instruction

  std::optional<Isa_extension> missing_extension{};

  switch (opcode) {
    case Opcode::load:
      switch (get_funct3(instruction)) {
        case 0: return Concrete_instruction::instr_lb;
        case 1: return Concrete_instruction::instr_lh;
        case 2: return Concrete_instruction::instr_lw;
        case 4: return Concrete_instruction::instr_lbu;
        case 5: return Concrete_instruction::instr_lhu;
      }
      break;
    case Opcode::op_imm:
      switch (get_funct3(instruction)) {
        case 0: return Concrete_instruction::instr_addi;
        case 1: return Concrete_instruction::instr_slli;
        case 2: return Concrete_instruction::instr_slti;
        case 3: return Concrete_instruction::instr_sltiu;
        case 4: return Concrete_instruction::instr_xori;
        case 5:
          switch (get_funct7(instruction)) {
            case 0        : return Concrete_instruction::instr_srli;
            case 0b0100000: return Concrete_instruction::instr_srai;
          }
          break;
        case 6: return Concrete_instruction::instr_ori;
        case 7: return Concrete_instruction::instr_addi;
      }
      break;
    case Opcode::auipc:
      return Concrete_instruction::instr_auipc;
      break;
    case Opcode::store:
      switch (get_funct3(instruction)) {
        case 0: return Concrete_instruction::instr_sb;
        case 1: return Concrete_instruction::instr_sh;
        case 2: return Concrete_instruction::instr_sw;
      }
      break;
    case Opcode::op:
      switch (get_funct3(instruction)) {
        case 0:
          switch (get_funct7(instruction)) {
            case 0        : return Concrete_instruction::instr_add;
            case 0b0100000: return Concrete_instruction::instr_sub;
          }
          break;
        case 1: return Concrete_instruction::instr_sll;
        case 2: return Concrete_instruction::instr_slt;
        case 3: return Concrete_instruction::instr_sltu;
        case 4: return Concrete_instruction::instr_xor;
        case 5:
          switch (get_funct7(instruction)) {
            case 0        : return Concrete_instruction::instr_srl;
            case 0b0100000: return Concrete_instruction::instr_sra;
          }
          break;
        case 6: return Concrete_instruction::instr_or;
        case 7: return Concrete_instruction::instr_and;
      }
      break;
    case Opcode::lui:
      return Concrete_instruction::instr_lui;
      break;
    case Opcode::branch:
      switch (get_funct3(instruction)) {
        case 0: return Concrete_instruction::instr_beq;
        case 1: return Concrete_instruction::instr_bne;
        case 4: return Concrete_instruction::instr_blt;
        case 5: return Concrete_instruction::instr_bge;
        case 6: return Concrete_instruction::instr_bltu;
        case 7: return Concrete_instruction::instr_bgeu;
      }
      break;
    case Opcode::jalr:
      if(get_funct3(instruction) == 0) return Concrete_instruction::instr_jalr;
      break;
    case Opcode::jal:
      return Concrete_instruction::instr_jal;
      break;
    case Opcode::misc_mem:
      if (get_funct3(instruction) == 0) return Concrete_instruction::instr_fence;
      break;
    case Opcode::system:
      switch (const auto funct3 = get_funct3(instruction)) {
        case 0:
          if (extract_bits(instruction, {31, 7}) == 0b0011000000100000000000000) {
            return Concrete_instruction::instr_mret;
          }
          break;
        case 4: break;
        default: // the rest are csr
          if (isa_extensions[Isa_extension::isa_zicsr]) {
            switch(funct3) {
              case 1: return Concrete_instruction::instr_csrrw;
              case 2: return Concrete_instruction::instr_csrrs;
              case 3: return Concrete_instruction::instr_csrrc;
              case 5: return Concrete_instruction::instr_csrrwi;
              case 6: return Concrete_instruction::instr_csrrsi;
              case 7: return Concrete_instruction::instr_csrrci;
            }
          } else {
            missing_extension = Isa_extension::isa_zicsr;
          }
          break;
      }
  }

  if (missing_extension) {
    throw Errors::Illegal_instruction{instruction, "From extension " + to_string(*missing_extension)};
  } else {
    throw Errors::Illegal_instruction{instruction};
  }
}

constexpr Decoder::Instruction_info Decoder::decode(Uxlen instruction) {
  Instruction_info info{};

  info.instruction = decode_concrete_instruction(instruction);
  decode_instruction_type(info);

  return info;
}

Decoder::Decoder(Callbacks callbacks, Uxlen instruction)
    : m_callbacks{callbacks}, m_instruction{instruction} {
  const Opcode opcode{static_cast<Opcode>(extract_bits(instruction, 2, 6))};

  const auto funct3{static_cast<unsigned int>(extract_bits(instruction, 12, 14))};
  const auto funct7{static_cast<unsigned int>(extract_bits(instruction, 25, 31))};

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
      callbacks.op(alu_op, ra1, ra2, wa);
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
      callbacks.op_imm(alu_op, ra1, get_imm_i(), wa);
    } break;

    case Opcode::lui : callbacks.lui(get_imm_u(), wa); break;

    case Opcode::load: {
      if (((funct3 & 0b11) == 0b11) || funct3 == 6) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::load"};
      }
      callbacks.load(funct3, ra1, get_imm_i(), wa);
    } break;

    case Opcode::store: {
      if (funct3 >= 3) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::store"};
      }
      callbacks.store(funct3, ra1, ra2, get_imm_i());
    } break;

    case Opcode::branch: {
      if ((funct3 == 2) || (funct3 == 3)) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::branch"};
      }
      callbacks.branch(static_cast<Alu::Op>((0b11 << 3) | funct3), ra1, ra2, get_imm_b());
    } break;

    case Opcode::jal: callbacks.jal(); break;

    case Opcode::jalr: {
      if (funct3 != 0) {
        throw Errors::Illegal_instruction{instruction, "illegal funct3 for Opcode::jalr"};
      }

      callbacks.jalr(ra1, get_imm_j(), wa);
    } break;

    case Opcode::auipc: callbacks.auipc(get_imm_u(), wa); break;

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
      callbacks.system(static_cast<Csr::Op>(funct3), ra1, wa, mret, get_imm_zicsr(), csr_we, gpr_we);
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
