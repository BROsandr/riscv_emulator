#pragma once

#include "memory.hpp"

#include <map>

class Csr : public Memory {
  public:
    enum class Register {
      MEPC     = 0x341,
      MIE      = 0x304,
      MTVEC    = 0x305,
      MSCRATCH = 0x340,
      MCAUSE   = 0x342,
    };

    enum class Op {
      CSR_RW  = 0b001,
      CSR_RS  = 0b010,
      CSR_RC  = 0b011,
      CSR_RWI = 0b101,
      CSR_RSI = 0b110,
      CSR_RCI = 0b111
    };

    Uxlen do_op(Op op, std::size_t addr, Uxlen data);

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override;
    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override;

  private:
    std::map<Register, Uxlen> registers;
};
