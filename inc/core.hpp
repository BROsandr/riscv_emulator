#pragma once

#include "isa_extension.hpp"
#include "riscv.hpp"
#include "memory.hpp"

#include <functional>
#include <memory>

#include <cassert>

namespace spdlog {
  class logger;
}

class Core {
  public:
    Core(const Memory &instr_mem, Memory &data_mem, Memory &csr, Memory &rf,
        std::shared_ptr<spdlog::logger> logger, Isa_ext_container isa_ext_container = {})
        : m_instr_mem{instr_mem}, m_data_mem{data_mem}, m_csr{csr}, m_rf{rf}, m_logger{logger},
        m_isa_ext_container{isa_ext_container} {
      assert(m_logger && "logger == nullptr in core");
    }
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
    const Memory &m_instr_mem;
    Memory &m_data_mem;
    Memory &m_csr;
    Memory &m_rf;
    std::shared_ptr<spdlog::logger> m_logger{nullptr};
    Uxlen m_pc{0};
    [[nodiscard]] Uxlen fetch_instruction() const {
      return m_data_mem.read(m_pc);
    }
    const Isa_ext_container m_isa_ext_container;

    void increment_pc();
};
