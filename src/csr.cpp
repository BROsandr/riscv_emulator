#include "csr.hpp"

#include "exception.hpp"

namespace {

  template<typename Int_t>
  constexpr bool is_legal_reg(Int_t reg) {
    using enum Csr::Register;
    switch (reg) {
      case Int_t(MEPC):
      case Int_t(MIE):
      case Int_t(MTVEC):
      case Int_t(MSCRATCH):
      case Int_t(MCAUSE): return true;

      default: return false;
    }
  }

  template<typename Int_t>
  constexpr void assert_legal_reg(Int_t reg) {
    if (!is_legal_reg(reg)) throw Errors::Illegal_addr{reg, "Illegal csr register."};
  }
}

void Csr::write(std::size_t addr, Uxlen data, unsigned int byte_en) {
  assert_legal_reg(addr);

  Register reg{static_cast<Register>(addr)};

  m_registers[reg] = data;
}

Uxlen Csr::read(std::size_t addr, unsigned int byte_en) {
  assert_legal_reg(addr);

  Register reg{static_cast<Register>(addr)};

  try {
    return m_registers.at(reg);
  } catch (const std::out_of_range &) {
    throw Errors::Illegal_addr(static_cast<std::size_t>(reg), "Read register was never written.");
  }
}
