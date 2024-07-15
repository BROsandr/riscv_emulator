#pragma once

#include "riscv.hpp"

#include <vector>

class Memory {
  public:
    virtual void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) = 0;
    virtual Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) = 0;

    virtual ~Memory() = default;
};

class Rf : public Memory {
  public:
    Rf(unsigned int registers_number = 32) : registers(registers_number, 0) {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      registers[addr] = data;
    }
    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      return registers[addr];
    }

  private:
    std::vector<Uxlen> registers;
};
