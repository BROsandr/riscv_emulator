#include "memory.hpp"
#include "riscv_algos.hpp"
#include "exception.hpp"

#include <stdexcept>
#include <utility>

template <typename Container> requires requires (Container cont) {
  { cont[0] } -> std::convertible_to<std::byte>;
  { cont.at(0) } -> std::convertible_to<std::byte>;
  { std::as_const(cont).at(0) } -> std::convertible_to<std::byte>;
}
class Data_mem_view : public Memory {
  public:
    Data_mem_view(Container &container) : m_container{container} {}

    using value_type = std::byte;

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      assert(((byte_en > 0) && (byte_en <= 0xf)) && "illegal byte_en when writing to data_mem");
      const auto to_byte = [](Uxlen data_) constexpr {
        return value_type{static_cast<uint8_t>(data_)};
      };
      if (extract_bits(byte_en, 0)) {
        try_set(addr, to_byte(extract_bits(data, {7,0})));
      }
      if (extract_bits(byte_en, 1)) {
        try_set(addr+1, to_byte(extract_bits(data, {15,8})));
      }
      if (extract_bits(byte_en, 2)) {
        try_set(addr+2, to_byte(extract_bits(data, {23,16})));
      }
      if (extract_bits(byte_en, 3)) {
        try_set(addr+3, to_byte(extract_bits(data, {31,24})));
      }
    }

    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      assert(((byte_en > 0) && (byte_en <= 0xf)) && "illegal byte_en when writing to data_mem");
      const auto to_uxlen = [](value_type b) constexpr {
        return static_cast<Uxlen>(b);
      };
      Uxlen data{};
      if (extract_bits(byte_en, 0)) {
        data |= to_uxlen(try_get(addr));
      }
      if (extract_bits(byte_en, 1)) {
        data |= to_uxlen(try_get(addr+1)) << CHAR_BIT;
      }
      if (extract_bits(byte_en, 2)) {
        data |= to_uxlen(try_get(addr+2)) << 2*CHAR_BIT;
      }
      if (extract_bits(byte_en, 3)) {
        data |= to_uxlen(try_get(addr+3)) << 3*CHAR_BIT;
      }
      return data;
    }

  private:
    Container &m_container;

    constexpr void try_set(std::size_t addr, value_type val) {
      try {
        m_container.at(addr) = val;
      } catch (const std::out_of_range&) {
        throw Errors::Illegal_addr(addr, "write address is out of data_mem range.");
      }
    }
    constexpr value_type try_get(std::size_t addr) const {
      try {
        return std::as_const(m_container).at(addr);
      } catch (const std::out_of_range&) {
        throw Errors::Illegal_addr(addr, "read address is out of data_mem range.");
      }
    }
};
