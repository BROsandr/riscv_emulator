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

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override;
    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override;

  private:
    std::map<Register, Uxlen> registers;
};
