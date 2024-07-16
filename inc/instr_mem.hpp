#pragma once

#include "memory.hpp"
#include "riscv.hpp"
#include "exception.hpp"

#include <stdexcept>
#include <string>

template <typename Container> requires requires (Container cont) {
  { cont[0] } -> std::convertible_to<Uxlen>;
  { cont.at(0) } -> std::convertible_to<Uxlen>;
} class Instr_mem : public Memory {
  public:
    Instr_mem(Container &instr_container) : m_instr_container{instr_container} {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      throw Errors::Read_only{"Write into instr_mem"};
    }

    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      try {
        return m_instr_container.at(addr);
      } catch (const std::out_of_range&) {
        using std::to_string;
        throw Errors::Illegal_addr(addr, "requested address is out of instr_mem range.");
      }
    }

  private:
    Container &m_instr_container;
};
