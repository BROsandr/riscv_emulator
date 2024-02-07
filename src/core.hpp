#pragma once

#include "riscv.hpp"

#include <functional>

class Memory;

struct Rf {
  Uxlen read(std::size_t address);
  Uxlen write(std::size_t address, Uxlen data);
};

struct Csr {};

class Core {
  public:
    Core(Memory &instr_mem, Memory &data_mem, std::function<void()> irq_callback);
    ~Core() = default;

    void request_irq() { this->irq_req = true; }
    void cycle();

  private:
    Core(const Core&) = delete;
    Core& operator=(const Core& ) = delete;
    Core& operator=(      Core&&) = delete;

    std::function<void()> irq_callback{nullptr};
    void return_from_irq();
    bool irq_req{false};
    Memory &instr_mem;
    Memory &data_mem;
    Rf rf{};
    Csr csr{};
    Uxlen pc{0};

    void increment_pc();
};
