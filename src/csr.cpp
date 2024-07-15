#include "csr.hpp"

#include "exception.hpp"

#include <cassert>

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

  registers[reg] = data;
}

Uxlen Csr::read(std::size_t addr, unsigned int byte_en) {
  assert_legal_reg(addr);

  Register reg{static_cast<Register>(addr)};

  try {
    return registers.at(reg);
  } catch (const std::out_of_range &) {
    throw Errors::Illegal_addr(reg, "Read register was never written.");
  }
}

Uxlen Csr::do_op(Op op, std::size_t addr, Uxlen data) {
  using enum Csr::Op;
  Uxlen res{read(addr)};
  switch (op) {
    case CSR_RW: case CSR_RWI:
      write(addr, data);
      break;
    case CSR_RS: case CSR_RSI:
      write(addr, res | data);
      break;
    case CSR_RC: case CSR_RCI:
      write(addr, res & ~data);
      break;
    default: assert(0 && "Illegal csr op.");
  }
  return res;
}
