#include "alu.hpp"
#include "csr.hpp"
#include "riscv.hpp"

#include <functional>

class Decoder {
  public:
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

    struct Callback {
      static std::function<void(Alu::Op, std::size_t ra1, std::size_t ra2, std::size_t wa)> op;
    };

    // namespace Callback {
    //   std::function<void(int func7)> op;
    // }

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
  private:
    const Uxlen m_instruction;

    constexpr int func7() const;
    constexpr int func3() const;

    constexpr Uxlen get_imm_i    () const;
    constexpr Uxlen get_imm_u    () const;
    constexpr Uxlen get_imm_s    () const;
    constexpr Uxlen get_imm_b    () const;
    constexpr Uxlen get_imm_j    () const;
    constexpr Uxlen get_imm_zicsr() const;

};
