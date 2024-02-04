#pragma once

#include <functional>

class Memory;

class Core {
  public:
    Core(Memory &instr_mem, Memory &data_mem, std::function<void()> irq_callback);

    void request_irq() { this->irq_req = true; }
    void cycle();

  private:
    Core(const Core&) = delete;
    Core& operator=(const Core& ) = delete;
    Core& operator=(      Core&&) = delete;
    ~Core() = default;

    std::function<void()> irq_callback{nullptr};
    void return_from_irq();
    bool irq_req{false};
    Memory &instr_mem;
    Memory &data_mem;
};
