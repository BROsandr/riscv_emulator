#pragma once

#include "riscv.hpp"
#include "exception.hpp"

#include <vector>

class Memory {
  public:
    virtual void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) = 0;
    virtual Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) = 0;

    virtual ~Memory() = default;
};


class Ranged_mem_span : public Memory {
  public:
    Ranged_mem_span(Memory &memory, std::size_t size, std::size_t start_addr = 0)
        : m_memory{memory}, m_start_addr{start_addr}, m_size{size} {}
    Ranged_mem_span(const Ranged_mem_span& that) = default;
    Ranged_mem_span& operator=(Ranged_mem_span) = delete;
    Ranged_mem_span(Ranged_mem_span&&) = delete;

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      assert_inside_range(addr);
      m_memory.write(addr - m_start_addr, data, byte_en);
    }
    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      assert_inside_range(addr);
      return m_memory.read(addr - m_start_addr, byte_en);
    }

  private:
    constexpr void assert_inside_range(std::size_t addr) const {
      if ((addr < m_start_addr) || (addr >= (m_start_addr + m_size))) {
        throw Errors::Illegal_addr{addr, "addr is out of range. start_addr: " +
            std::to_string(m_start_addr) + ", size: " + std::to_string(m_size)};
      }
    }

    Memory &m_memory;
    const std::size_t m_start_addr;
    const std::size_t m_size;
    constexpr std::size_t get_end_addr() const {
      return m_start_addr + m_size - 1;
    }
};

class Rf : public Memory {
  public:
    Rf(unsigned int registers_number = 32) : m_registers(registers_number, 0) {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      m_registers[addr] = data;
    }
    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      return m_registers[addr];
    }

  private:
    std::vector<Uxlen> m_registers;
};
