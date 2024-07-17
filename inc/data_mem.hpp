#include "memory.hpp"
#include "riscv_algos.hpp"
#include "exception.hpp"

template <typename Container> requires requires (Container cont) {
  { cont[0] } -> std::convertible_to<std::byte>;
  { cont.at(0) } -> std::convertible_to<std::byte>;
}
class Data_mem_wrap : public Memory {
  public:
    Data_mem_wrap(Container &container) : m_container{container} {}

    void write(std::size_t addr, Uxlen data, unsigned int byte_en = 0xf) override {
      if (extract_bits(byte_en, 0)) {
        m_container[addr] = extract_bits(data, {7,0});
      }
      if (extract_bits(byte_en, 1)) {
        m_container[addr+1] = extract_bits(data, {15,8});
      }
      if (extract_bits(byte_en, 2)) {
        m_container[addr+2] = extract_bits(data, {23,16});
      }
      if (extract_bits(byte_en, 3)) {
        m_container[addr+3] = extract_bits(data, {31,24});
      }
    }

    Uxlen read (std::size_t addr, unsigned int byte_en = 0xf) override {
      try {
        const auto &const_container{m_container};
        const Uxlen data =  (const_container.at(addr  ) << CHAR_BIT) |
                            (const_container.at(addr+1) << CHAR_BIT) |
                            (const_container.at(addr+2) << CHAR_BIT) |
                            (const_container.at(addr+3)            );
        return data;
      } catch (const std::out_of_range&) {
        using std::to_string;
        throw Errors::Illegal_addr(addr, "requested address is out of data_mem range.");
      }
    }

  private:
    Container &m_container;
};
