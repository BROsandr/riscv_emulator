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
    using enum Decoder::Instruction_type;
    switch (info.get_type()) {
      case none :                     break;
      case i    : decode_i    (info, instruction); break;
      case i_sh5: decode_i_sh5(info, instruction); break;
      case r    : decode_r    (info, instruction); break;
      case s    : decode_s    (info, instruction); break;
      case u    : decode_u    (info, instruction); break;
      case uj   : decode_uj   (info, instruction); break;
      case sb   : decode_sb   (info, instruction); break;
      default   : assert(0 && "Unexpected instruction type");
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
          if (m_isa_ext_container[Isa_extension::isa_zicsr]) {
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
