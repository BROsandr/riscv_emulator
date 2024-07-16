#pragma once

#include "riscv.hpp"
#include "memory.hpp"

#include <functional>

class Core {
  public:
    Core(Memory &instr_mem, Memory &data_mem, Memory &csr, Memory &rf)
        : m_instr_mem{instr_mem}, m_data_mem{data_mem}, m_csr{csr}, m_rf{rf} {}
    ~Core() = default;

    void request_irq() { this->m_irq_req = true; }
    void cycle();
    std::function<void()> m_irq_callback{nullptr};
    std::function<void()> m_return_from_irq{nullptr};

  private:
    Core(const Core&) = delete;
    Core& operator=(const Core& ) = delete;
    Core& operator=(      Core&&) = delete;

    bool m_irq_req{false};
    Memory &m_instr_mem;
    Memory &m_data_mem;
    Memory &m_csr;
    Memory &m_rf;
    Uxlen m_pc{0};
    constexpr Uxlen fetch_instruction() const {
      return m_data_mem.read(m_pc);
    }

    void increment_pc();
};
