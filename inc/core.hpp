#pragma once

#include "riscv.hpp"
#include "memory.hpp"

#include <functional>

class Core {
  public:
    Core(Memory &instr_mem, Memory &data_mem, Memory &csr, Memory &rf)
        : m_instr_mem{instr_mem}, m_data_mem{data_mem}, m_csr{csr}, m_rf{rf} {}
    ~Core() = default;

    void request_irq() { this->irq_req = true; }
    void cycle();

    Core& set_irq_callback(std::function<void()> irq_callback) {
      m_irq_callback = irq_callback;
      return *this;
    }

    std::function<void()> get_irq_callback() const {
      return m_irq_callback;
    }

    Core& set_return_from_irq(std::function<void()> return_from_irq) {
      m_return_from_irq = return_from_irq;
      return *this;
    }

    std::function<void()> get_return_from_irq() const {
      return m_return_from_irq;
    }

  private:
    Memory& get_rf() const {
      return m_rf;
    }
    Memory& get_csr() const {
      return m_csr;
    }
    Memory& get_data_mem() const {
      return m_data_mem;
    }
    Memory& get_instr_mem() const {
      return m_instr_mem;
    }
    Uxlen& get_pc() {
      return m_pc;
    }
    Uxlen get_pc() const {
      return m_pc;
    }

    Core(const Core&) = delete;
    Core& operator=(const Core& ) = delete;
    Core& operator=(      Core&&) = delete;

    std::function<void()> m_irq_callback{nullptr};
    std::function<void()> m_return_from_irq{nullptr};
    bool irq_req{false};
    Memory &m_instr_mem;
    Memory &m_data_mem;
    Memory &m_csr;
    Memory &m_rf;
    Uxlen m_pc{0};
    constexpr Uxlen fetch_instruction() const {
      return get_data_mem().read(get_pc());
    }

    void increment_pc();
};
