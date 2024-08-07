#pragma once

#include "memory.hpp"
#include "riscv.hpp"
#include "exception.hpp"

#include <cassert>

#include <stdexcept>
#include <string>
#include <utility>

template <typename Container> requires requires (Container cont) {
  { std::as_const(cont).at(0) } -> std::convertible_to<Uxlen>;
}
class Instr_mem : public Memory {
  public:
    using value_type = Uxlen;

    Instr_mem(Container content) : m_content{std::move(content)} {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      throw Errors::Read_only{"Write into instr_mem"};
    }

    [[nodiscard]] Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      assert((byte_en == 0xf) && "only word access is supported");
      assert(!(addr & 0b11) && "only alignment by word is supported");
      const std::size_t word_addr{addr >> 2};
      return try_get(word_addr);
    }

    [[nodiscard]] const Container& get_content() const {
      return m_content;
    }

  private:
    const Container m_content;

    value_type try_get(std::size_t addr) const {
      try {
        return std::as_const(m_content).at(addr);
      } catch (const std::out_of_range&) {
        throw Errors::Illegal_addr(addr, "read address is out of instr_mem range.");
      }
    }
};
