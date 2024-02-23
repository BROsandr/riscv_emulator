#pragma once

#include "riscv.hpp"

#include <bitset>

class Decoder {
  public:
    enum Isa_extension {
      isa_zicsr,
      isa_number_
    };

    using Isa_extensions = std::bitset<static_cast<std::size_t>(Isa_extension::isa_number_)>;

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
      std::size_t          rs1        {};
      std::size_t          rs2        {};
      std::size_t          rs3        {};
      Imm                  imm        {};
      std::size_t          rd         {};
      Concrete_instruction instruction{};
      Instruction_type     type       {};
    };

    constexpr Decoder(Isa_extensions extensions) : isa_extensions{extensions} {};
    constexpr Decoder(Isa_extension  extension)  : Decoder{Isa_extensions{}.set(extension)} {};
    constexpr Instruction_info decode(Uxlen instruction);

  private:
    const Isa_extensions isa_extensions;

    constexpr Concrete_instruction decode_concrete_instruction(Uxlen instruction);

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
