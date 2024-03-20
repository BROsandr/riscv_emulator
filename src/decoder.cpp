#include "decoder.hpp"

#include "exception.hpp"
#include "riscv_algos.hpp"

#include <optional>
#include <type_traits>

namespace {
  constexpr unsigned int get_funct3(Uxlen instruction) {
    return static_cast<unsigned int>(extract_bits(instruction, {14, 12}));
  }

  constexpr unsigned int get_funct7(Uxlen instruction) {
    return static_cast<unsigned int>(extract_bits(instruction, {31, 25}));
  }

  constexpr unsigned int get_rd(Uxlen instruction) {
    return extract_bits(instruction, {11, 7});
  }

  constexpr unsigned int get_rs1(Uxlen instruction) {
    return extract_bits(instruction, {19, 15});
  }

  constexpr unsigned int get_rs2(Uxlen instruction) {
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

Decoder::Concrete_instruction Decoder::decode_concrete_instruction(Uxlen instruction)
    const {
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
        case 7: return Concrete_instruction::instr_andi;
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

Decoder::Instruction_info Decoder::decode(Uxlen instruction) const {
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
      try {
        decoder.decode(0x30529073);
      } catch (const Errors::Illegal_instruction &exception) {
        REQUIRE(exception.m_instruction == 0x30529073);
      }
    }
  }
}

TEST_CASE("Decoder lui", "[LUI]") {
  Decoder decoder{};
  SECTION("lui x27, 50") {
    Decoder::Instruction_info info{decoder.decode(0x00032db7)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_lui);
    REQUIRE(info.type == Decoder::Instruction_type::u);
    REQUIRE(info.rd   == 27);
    REQUIRE(info.imm  == 50);
  }
}

TEST_CASE("Decoder auipc", "[AUIPC]") {
  Decoder decoder{};
  SECTION("auipc x7, 379") {
    Decoder::Instruction_info info{decoder.decode(0x0017b397)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_auipc);
    REQUIRE(info.type == Decoder::Instruction_type::u);
    REQUIRE(info.rd   == 7);
    REQUIRE(info.imm  == 379);
  }
}

TEST_CASE("Decoder slti", "[SLTI]") {
  Decoder decoder{};
  SECTION("slti x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dba613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_slti);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder sltiu", "[SLTIU]") {
  Decoder decoder{};
  SECTION("sltiu x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbb613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_sltiu);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder xori", "[XORI]") {
  Decoder decoder{};
  SECTION("xori x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbc613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_xori);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder ori", "[ORI]") {
  Decoder decoder{};
  SECTION("ori x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbe613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_ori);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder andi", "[ANDI]") {
  Decoder decoder{};
  SECTION("andi x12, x23, 413") {
    Decoder::Instruction_info info{decoder.decode(0x19dbf613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_andi);
    REQUIRE(info.type == Decoder::Instruction_type::i);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 413);
  }
}

TEST_CASE("Decoder slli", "[SLLI]") {
  Decoder decoder{};
  SECTION("slli x12, x23, 31") {
    Decoder::Instruction_info info{decoder.decode(0x01fb9613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_slli);
    REQUIRE(info.type == Decoder::Instruction_type::i_sh5);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 31);
  }
}

TEST_CASE("Decoder srli", "[SRLI]") {
  Decoder decoder{};
  SECTION("srli x12, x23, 31") {
    Decoder::Instruction_info info{decoder.decode(0x01fbd613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_srli);
    REQUIRE(info.type == Decoder::Instruction_type::i_sh5);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 31);
  }
}

TEST_CASE("Decoder srai", "[SRAI]") {
  Decoder decoder{};
  SECTION("srai x12, x23, 31") {
    Decoder::Instruction_info info{decoder.decode(0x41fbd613)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_srai);
    REQUIRE(info.type == Decoder::Instruction_type::i_sh5);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.imm  == 31);
  }
}

TEST_CASE("Decoder sub", "[SUB]") {
  Decoder decoder{};
  SECTION("sub x12, x23, x15") {
    Decoder::Instruction_info info{decoder.decode(0x40fb8633)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_sub);
    REQUIRE(info.type == Decoder::Instruction_type::r);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.rs2  == 15);
  }
}

TEST_CASE("Decoder sll", "[SLL]") {
  Decoder decoder{};
  SECTION("sll x12, x23, x15") {
    Decoder::Instruction_info info{decoder.decode(0x00fb9633)};
    REQUIRE(info.instruction == Decoder::Concrete_instruction::instr_sll);
    REQUIRE(info.type == Decoder::Instruction_type::r);
    REQUIRE(info.rd   == 12);
    REQUIRE(info.rs1  == 23);
    REQUIRE(info.rs2  == 15);
  }
}
#endif
