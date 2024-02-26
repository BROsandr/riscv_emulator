#pragma once

#include "riscv.hpp"

#include <vector>

class Memory {
  public:
    using Word = Uxlen;

    virtual void write(std::size_t addr, Uxlen data)       = 0;
    virtual Word read (std::size_t addr)             const = 0;

    virtual ~Memory() = default;
};

class Rf : public Memory {
  public:
    Rf(std::size_t registers_number = 32) : registers(registers_number, 0) {}

    void write(std::size_t addr, Uxlen data) override {
      registers.at(addr) = data;
    }
    Word read (std::size_t addr) const override {
      return registers.at(addr);
    }

  private:
    std::vector<Uxlen> registers;
};
