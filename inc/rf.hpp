#pragma once

#include "memory.hpp"

#include <vector>

class Rf : public Memory {
  public:
    using Container = std::vector<Uxlen>;

    Rf(unsigned int registers_number = 32) : m_registers(registers_number, 0) {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      m_registers[addr] = data;
    }
    [[nodiscard]] Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      return m_registers[addr];
    }

    [[nodiscard]] const Container& get_content() const {
      return m_registers;
    }

  private:
    Container m_registers;
};
