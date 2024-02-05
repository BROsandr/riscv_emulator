#include "alu.hpp"
#include "csr.hpp"
#include "riscv.hpp"

class Decoder {
  public:
    enum class Opcode {
      load_opcode     = 0b00000,
      misc_mem_opcode = 0b00011,
      op_imm_opcode   = 0b00100,
      auipc_opcode    = 0b00101,
      store_opcode    = 0b01000,
      op_opcode       = 0b01100,
      lui_opcode      = 0b01101,
      branch_opcode   = 0b11000,
      jalr_opcode     = 0b11001,
      jal_opcode      = 0b11011,
      system_opcode   = 0b11100
    };

    enum class B_sel {
      rd,
      imm_i,
      imm_u,
      imm_s
    };

    constexpr Decoder(Uxlen instruction);

    constexpr B_sel   get_b_sel () const;
    constexpr Alu::Op get_alu_op() const;
    constexpr Csr::Op get_csr_op() const;
    constexpr bool    get_csr_we() const;

};
