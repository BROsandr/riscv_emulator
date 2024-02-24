#include "decoder.hpp"

#include "exception.hpp"

#include <cassert>
#include <climits>

#include <initializer_list>
#include <iostream>
#include <optional>
#include <type_traits>

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

  template <typename T>
  constexpr T sign_extend(T data, std::size_t sign_pos) {
    assert(sign_pos < (sizeof(data) * 8));

    T m{T{1} << (sign_pos-1)};
    return (data ^ m) - m;
  }

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, Bit_range range, bool sext = false) {
    assert(range.get_msb() < (sizeof(T) * CHAR_BIT));
    T unextended{(data >> range.get_lsb()) & static_cast<T>(make_mask<unsigned long long>(range.get_width()))};
    if (sext) return sign_extend(unextended, range.get_width() - 1);
    else      return unextended;
  }

  template <typename T> requires Right_shiftable<T> && Andable<T>
  constexpr T extract_bits(T data, std::size_t pos, bool sext = false) {
    return extract_bits(data, {pos, pos}, sext);
  }

  template <typename T>
  concept Left_shiftable = requires(T a) {
    a << 1;
  };

  template <typename T>
  concept Shiftable = Left_shiftable<T> && Right_shiftable<T>;

  template <typename T>
  concept Orable = requires(T a) {
    a | static_cast<T>(1);
  };

  template <typename T> requires Shiftable<T> && Orable<T>
  constexpr T extract_bits(T data, std::initializer_list<Bit_range> bit_ranges, bool sext = false) {
    T result{extract_bits(data, *(bit_ranges.begin()), sext)};
    for (const auto *it{bit_ranges.begin() + 1}; it != bit_ranges.end(); ++it) {
      result = (result << (*it).get_width()) | extract_bits(data, *it);
    }
    return result;
  }

  constexpr unsigned int get_funct3(Uxlen instruction) {
    return static_cast<unsigned int>(extract_bits(instruction, {14, 12}));
  }

  constexpr unsigned int get_funct7(Uxlen instruction) {
    return static_cast<unsigned int>(extract_bits(instruction, {31, 25}));
  }

  constexpr std::size_t get_rd(Uxlen instruction) {
    return extract_bits(instruction, {11, 7});
  }

  constexpr std::size_t get_rs1(Uxlen instruction) {
    return extract_bits(instruction, {19, 15});
  }

  constexpr std::size_t get_rs2(Uxlen instruction) {
    return extract_bits(instruction, {24, 20});
  }

  constexpr Uxlen get_imm20(Uxlen instruction) {
    return extract_bits(instruction, {31, 12});
  }

  constexpr Uxlen get_jimm20(Uxlen instruction) {
    return extract_bits(instruction, {Bit_range{20}, {10, 1}, Bit_range{11}, {19, 12}});
  }

  constexpr Uxlen get_imm12(Uxlen instruction) {
    return extract_bits(instruction, {31, 20});
  }

  constexpr Uxlen get_shamt5(Uxlen instruction) {
    return extract_bits(instruction, {24, 20});
  }

  constexpr Uxlen get_simm12(Uxlen instruction) {
    return extract_bits(instruction, {{11, 5}, {4, 0}}, true);
  }

  constexpr Uxlen get_sbimm12(Uxlen instruction) {
    return extract_bits(instruction, {Bit_range{12}, {10, 5}, {4, 1}, Bit_range{11}}, true);
  }

  constexpr std::string to_string(Isa_extension extension) {
    using enum Isa_extension;
    switch (extension) {
      case isa_zicsr: return "Zicsr";
      default: assert((void("Unknown isa_extension" + std::to_string(
          static_cast<std::underlying_type_t<Isa_extension>>(extension))), 0)
      );
    }
  }

  constexpr Decoder::Instruction_type concrete2type(Decoder::Concrete_instruction instruction) {
    using enum Decoder::Concrete_instruction;
    using enum Decoder::Instruction_type;
    switch (instruction) {
      case instr_lui   :
      case instr_auipc : return u;

      case instr_jal   : return uj;

      case instr_jalr  :
      case instr_lb    :
      case instr_lh    :
      case instr_lw    :
      case instr_lbu   :
      case instr_lhu   :
      case instr_addi  :
      case instr_slti  :
      case instr_sltiu :
      case instr_xori  :
      case instr_ori   :
      case instr_andi  :
      case instr_csrrw :
      case instr_csrrs :
      case instr_csrrc :
      case instr_csrrwi:
      case instr_csrrsi:
      case instr_csrrci: return i;

      case instr_beq   :
      case instr_bne   :
      case instr_blt   :
      case instr_bge   :
      case instr_bltu  :
      case instr_bgeu  : return sb;

      case instr_sb    :
      case instr_sh    :
      case instr_sw    : return s;

      case instr_slli  :
      case instr_srli  :
      case instr_srai  : return i_sh5;

      case instr_add   :
      case instr_sub   :
      case instr_sll   :
      case instr_slt   :
      case instr_sltu  :
      case instr_xor   :
      case instr_srl   :
      case instr_sra   :
      case instr_or    :
      case instr_and   : return r;

      case instr_fence :
      case instr_mret  : return none;
    }

    assert((void("Unknown instruction" + std::to_string(instruction)),0));
  }

  constexpr void decode_i   (Decoder::Instruction_info &info, Uxlen instr) {
    info.rd  = get_rd(instr);
    info.rs1 = get_rs1(instr);
    info.imm = get_imm12(instr);
  }
  constexpr void decode_i_sh5(Decoder::Instruction_info &info, Uxlen instr) {
    info.rd  = get_rd(instr);
    info.rs1 = get_rs1(instr);
    info.imm = get_shamt5(instr);
  }
  constexpr void decode_r    (Decoder::Instruction_info &info, Uxlen instr) {
    info.rd  = get_rd(instr);
    info.rs1 = get_rs1(instr);
    info.rs2 = get_rs2(instr);
  }
  constexpr void decode_s    (Decoder::Instruction_info &info, Uxlen instr) {
    info.rs1 = get_rs1(instr);
    info.rs2 = get_rs2(instr);
    info.imm = get_simm12(instr);
  }
  constexpr void decode_u    (Decoder::Instruction_info &info, Uxlen instr) {
    info.rd  = get_rd(instr);
    info.rs1 = get_rs1(instr);
    info.imm = get_imm20(instr);
  }
  constexpr void decode_uj   (Decoder::Instruction_info &info, Uxlen instr) {
    info.rd  = get_rd(instr);
    info.rs1 = get_rs1(instr);
    info.imm = get_jimm20(instr);
  }
  constexpr void decode_sb   (Decoder::Instruction_info &info, Uxlen instr) {
    info.rs1 = get_rs1(instr);
    info.rs2 = get_rs2(instr);
    info.imm = get_sbimm12(instr);
  }

  constexpr void decode_instruction_type(Decoder::Instruction_info &info, Uxlen instruction) {
    info.type = concrete2type(info.instruction);
    using enum Decoder::Instruction_type;
    switch (info.type) {
      case none :                     break;
      case i    : decode_i    (info, instruction); break;
      case i_sh5: decode_i_sh5(info, instruction); break;
      case r    : decode_r    (info, instruction); break;
      case s    : decode_s    (info, instruction); break;
      case u    : decode_u    (info, instruction); break;
      case uj   : decode_uj   (info, instruction); break;
      case sb   : decode_sb   (info, instruction); break;
      default   : assert(0 && "Unexpected info.type");
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
          if (isa_ext_container[Isa_extension::isa_zicsr]) {
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
  decode_instruction_type(info, instruction);

  return info;
}

#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Decoder add", "[ADD]") {
  Decoder decoder{};
  SECTION("add x1, x2, x3") {
    Decoder::Instruction_info info{decoder.decode(0x003100b3)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_add);
    REQUIRE(info.type == Decoder::Instruction_type::r);
    REQUIRE(info.rd   == 1);
    REQUIRE(info.rs1  == 2);
    REQUIRE(info.rs2  == 3);
  }
}

TEST_CASE("Decoder addi", "[ADDI]") {
  Decoder decoder{};
  SECTION("addi x1, x2, 256") {
    Decoder::Instruction_info info{decoder.decode(0x10010093)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_addi);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 1);
    REQUIRE(info.rs1  == 2);
    REQUIRE(info.imm  == 256);
  }
}

TEST_CASE("Decoder csrw", "[CSRW]") {
  SECTION("enabled zicsr") {
    Decoder decoder{Isa_extension::isa_zicsr};
    SECTION("nothrow") {
      REQUIRE_NOTHROW(decoder.decode(0x30529073));
    }
    SECTION("csrw mtvec, x5") {
      Decoder::Instruction_info info{decoder.decode(0x30529073)};
      REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_csrrw);
      REQUIRE(info.type == Decoder::Instruction_type::i);
      REQUIRE(info.rs1  == 5);
      REQUIRE(info.imm  == 0x305);
    }
  }
  SECTION("disabled zicsr") {
    Decoder decoder{};
    SECTION("throws") {
      REQUIRE_THROWS(decoder.decode(0x30529073));
    }
    SECTION("throws instruction content check") {
      try {
        decoder.decode(0x30529073);
      } catch (const Errors::Illegal_instruction &exception) {
        REQUIRE(exception.m_instruction == 0x30529073);
      }
    }
  }
}
#endif
