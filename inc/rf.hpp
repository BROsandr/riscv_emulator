#pragma once

#include "memory.hpp"

#include <vector>

class Rf : public Memory {
  public:
    Rf(unsigned int registers_number = 32) : m_registers(registers_number, 0) {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      m_registers[addr] = data;
    }
    [[nodiscard]] Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      return m_registers[addr];
    }

  private:
    std::vector<Uxlen> m_registers;
};
