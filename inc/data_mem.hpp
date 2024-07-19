#include "memory.hpp"
#include "riscv_algos.hpp"
#include "exception.hpp"

#include <stdexcept>
#include <string>
#include <utility>

template <typename Container> requires requires (Container cont) {
  { cont[0] } -> std::convertible_to<std::byte>;
  { cont.at(0) } -> std::convertible_to<std::byte>;
  { std::as_const(cont).at(0) } -> std::convertible_to<std::byte>;
}
class Data_mem_view : public Memory {
  public:
    Data_mem_view(Container &container) : m_container{container} {}
    Data_mem_view(const Data_mem_view&) = default;
    Data_mem_view& operator=(Data_mem_view) = delete;
    Data_mem_view(Data_mem_view&&) = delete;

    using value_type = std::byte;

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      if(m_assured_aligment && is_misaliged(addr, byte_en)) {
        throw Errors::Misalignment{addr, "illegal byte_en (" +
            std::to_string(byte_en) + ") when reading from data_mem"};
      }
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
      if(m_assured_aligment && is_misaliged(addr, byte_en)) {
        throw Errors::Misalignment{addr, "illegal byte_en (" +
            std::to_string(byte_en) + ") when writing to data_mem"};
      }
      const auto to_uxlen = [](value_type b) constexpr {
        return static_cast<Uxlen>(b);
      };
      Uxlen data{0};
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
    bool m_assured_aligment{true};

  private:
    Container &m_container;

    constexpr bool is_misaliged(std::size_t addr, unsigned int byte_en = 0xf) const {
      return !((byte_en > 0) && (byte_en <= 0xf));
    }

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

template <typename Viewed>
class Ranged_view {
  public:
    Ranged_view(Viewed &viewed, std::size_t start_addr, std::size_t size)
        : m_viewed{viewed}, m_start_addr{start_addr}, m_size{size} {}
    Ranged_view(const Ranged_view&) = default;
    Ranged_view& operator=(Ranged_view) = delete;
    Ranged_view(Ranged_view&&) = delete;

    auto& operator[](const std::size_t& addr) {
      assert_inside_range(addr);
      return m_viewed[addr];
    }
    auto& operator[](std::size_t&& addr) {
      assert_inside_range(addr);
      return m_viewed[std::move(addr)];
    }

    auto& at(const std::size_t& addr) {
      assert_inside_range(addr);
      return m_viewed[addr];
    }
    const auto& at(const std::size_t& addr) const {
      assert_inside_range(addr);
      return m_viewed.at(addr);
    }

  private:
    constexpr void assert_inside_range(std::size_t addr) const {
      if ((addr < m_start_addr) || (addr >= (m_start_addr + m_size))) {
        throw Errors::Illegal_addr{addr, "addr is out of range. start_addr: " +
            std::to_string(m_start_addr) + ", size: " + std::to_string(m_size)};
      }
    }

    Viewed &m_viewed;
    const std::size_t m_start_addr;
    const std::size_t m_size;
};
