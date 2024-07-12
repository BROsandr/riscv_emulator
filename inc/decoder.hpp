#pragma once

#include "riscv.hpp"

#include "isa_extension.hpp"

#include <cassert>

class Decoder {
  public:

    enum class Instruction_type {
      r,
      i,
      s,
      u,
      uj,
      sb,
      i_sh5,
      none
    };

    enum Concrete_instruction {
      instr_lui   ,
      instr_auipc ,
      instr_jal   ,
      instr_jalr  ,
      instr_beq   ,
      instr_bne   ,
      instr_blt   ,
      instr_bge   ,
      instr_bltu  ,
      instr_bgeu  ,
      instr_lb    ,
      instr_lh    ,
      instr_lw    ,
      instr_lbu   ,
      instr_lhu   ,
      instr_sb    ,
      instr_sh    ,
      instr_sw    ,
      instr_addi  ,
      instr_slti  ,
      instr_sltiu ,
      instr_xori  ,
      instr_ori   ,
      instr_andi  ,
      instr_slli  ,
      instr_srli  ,
      instr_srai  ,
      instr_add   ,
      instr_sub   ,
      instr_sll   ,
      instr_slt   ,
      instr_sltu  ,
      instr_xor   ,
      instr_srl   ,
      instr_sra   ,
      instr_or    ,
      instr_and   ,
      instr_fence ,
      instr_mret  ,
      instr_csrrw ,
      instr_csrrs ,
      instr_csrrc ,
      instr_csrrwi,
      instr_csrrsi,
      instr_csrrci,
    };

    struct Instruction_info {
      unsigned int         rs1        {};
      unsigned int         rs2        {};
      unsigned int         rs3        {};
      Imm                  imm        {};
      unsigned int         rd         {};
      Concrete_instruction instruction{};

      constexpr Instruction_type get_type() const;
    };

    explicit constexpr Decoder(Isa_ext_container extensions)
        : isa_ext_container{extensions} {};
    constexpr Decoder() = default;
    Instruction_info decode(Uxlen instruction) const;

  private:
    const Isa_ext_container isa_ext_container{};

    Concrete_instruction decode_concrete_instruction(Uxlen instruction) const;

    enum class Opcode {
      load     = 0b00000,
      misc_mem = 0b00011,
      op_imm   = 0b00100,
      auipc    = 0b00101,
      store    = 0b01000,
      op       = 0b01100,
      lui      = 0b01101,
      branch   = 0b11000,
      jalr     = 0b11001,
      jal      = 0b11011,
      system   = 0b11100
    };

};

constexpr Decoder::Instruction_type Decoder::Instruction_info::get_type() const {
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
