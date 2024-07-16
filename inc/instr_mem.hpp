#pragma once

#include "memory.hpp"
#include "riscv.hpp"
#include "exception.hpp"

#include <string>

template <typename Iter>
class Instr_mem : public Memory {
  static_assert(std::random_access_iterator<Iter>);
  public:
    Instr_mem(Iter it, Iter end) : m_it{it}, m_end{end} {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      throw Errors::Read_only{"Write into instr_mem"};
    }

    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      const auto requested_it = m_it + addr;
      if (requested_it >= m_end) {
        using std::to_string;
        throw Errors::Illegal_addr(addr, "requested address exceeds instr_mem length (" +
            to_string(m_end - m_it) + ").");
      }
      return *requested_it;
    }

  private:
    Iter m_it;
    Iter m_end;
};
