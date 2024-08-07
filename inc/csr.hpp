#pragma once

#include "memory.hpp"

#include <map>

class Csr : public Memory {
  public:
    enum Register : std::size_t {
      MEPC     = 0x341,
      MIE      = 0x304,
      MTVEC    = 0x305,
      MSCRATCH = 0x340,
      MCAUSE   = 0x342,
    };

    using Container = std::map<Register, Uxlen>;

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override;
    [[nodiscard]] Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override;

    [[nodiscard]] const Container& get_content() const {
      return m_registers;
    }

  private:
    Container m_registers{};
};
