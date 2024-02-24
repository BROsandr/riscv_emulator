#pragma once

#include "riscv.hpp"

#include <initializer_list>
#include <bitset>
#include <algorithm>

enum class Isa_extension {
  isa_zicsr,
  isa_number_
};

class Isa_ext_container {
  private:
    using Base_bitset = std::bitset<static_cast<std::size_t>(Isa_extension::isa_number_)>;

  public:
    constexpr Isa_ext_container(std::initializer_list<Isa_extension> extensions) {
      std::for_each(extensions.begin(), extensions.end(),
          [this](const auto &extension) {
            set(extension);
          });
    }
    constexpr Isa_ext_container(Isa_extension extension)
        : Isa_ext_container({extension}) {}
    constexpr Isa_ext_container() = default;

    constexpr bool operator[](Isa_extension extension) const {
      return m_extensions[static_cast<std::size_t>(extension)];
    }
    constexpr Base_bitset::reference operator[](Isa_extension extension) {
      return m_extensions[static_cast<std::size_t>(extension)];
    }

    constexpr Isa_ext_container& set() {
      m_extensions.set();
      return *this;
    }
    constexpr Isa_ext_container& set(Isa_extension extension, bool value = true) {
      m_extensions.set(static_cast<std::size_t>(extension), value);
      return *this;
    }

  private:
    Base_bitset m_extensions{};
};

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
      std::size_t          rs1        {};
      std::size_t          rs2        {};
      std::size_t          rs3        {};
      Imm                  imm        {};
      std::size_t          rd         {};
      Concrete_instruction instruction{};
      Instruction_type     type       {};
    };

    explicit constexpr Decoder(Isa_ext_container extensions)
        : isa_ext_container{extensions} {};
    constexpr Decoder() = default;
    constexpr Instruction_info decode(Uxlen instruction);

  private:
    const Isa_ext_container isa_ext_container{};

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
