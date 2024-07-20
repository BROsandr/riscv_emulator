#include "memory.hpp"
#include "riscv_algos.hpp"
#include "exception.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <map>

class Data_mem : public Memory {
  public:
    using Map = std::map<std::size_t, std::byte>;
    Data_mem(Map container) : m_container{std::move(container)} {}

    using mapped_type = Map::mapped_type;

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      if(m_assured_aligment && is_misaliged(addr, byte_en)) {
        throw Errors::Misalignment{addr, "illegal byte_en (" +
            std::to_string(byte_en) + ") when reading from data_mem"};
      }
      const auto to_byte = [](Uxlen data_) constexpr {
        return mapped_type{static_cast<uint8_t>(data_)};
      };
      if (extract_bits(byte_en, 0)) {
        m_container.insert_or_assign(addr, to_byte(extract_bits(data, {7,0})));
      }
      if (extract_bits(byte_en, 1)) {
        m_container.insert_or_assign(addr+1, to_byte(extract_bits(data, {15,8})));
      }
      if (extract_bits(byte_en, 2)) {
        m_container.insert_or_assign(addr+2, to_byte(extract_bits(data, {23,16})));
      }
      if (extract_bits(byte_en, 3)) {
        m_container.insert_or_assign(addr+3, to_byte(extract_bits(data, {31,24})));
      }
    }

    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      if(m_assured_aligment && is_misaliged(addr, byte_en)) {
        throw Errors::Misalignment{addr, "illegal byte_en (" +
            std::to_string(byte_en) + ") when writing to data_mem"};
      }
      const auto to_uxlen = [](mapped_type b) constexpr {
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
    Map m_container;

    constexpr bool is_misaliged(std::size_t addr, unsigned int byte_en = 0xf) const {
      return !((byte_en > 0) && (byte_en <= 0xf));
    }

    mapped_type try_get(std::size_t addr) const {
      try {
        return std::as_const(m_container).at(addr);
      } catch (const std::out_of_range&) {
        throw Errors::Illegal_addr(addr, "read address is out of data_mem range.");
      }
    }
};
