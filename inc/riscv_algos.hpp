#pragma once

#include <cassert>
#include <climits>

#include <iostream>
#include <initializer_list>

class Bit_range {
  public:
    constexpr Bit_range(std::size_t msb, std::size_t lsb)
        : m_msb{msb}, m_lsb{lsb}, m_width{msb - lsb + 1} {
      assert(msb >= lsb);
    }
    explicit constexpr Bit_range(std::size_t pos)
        : Bit_range(pos, pos) {}

    constexpr std::size_t get_msb()   const {return m_msb;}
    constexpr std::size_t get_lsb()   const {return m_lsb;}
    constexpr std::size_t get_width() const {return m_width;}

    constexpr bool is_overlapping(const Bit_range& rhs) const {
      return get_lsb() <= rhs.get_msb() && rhs.get_lsb() <= get_msb();
    }

    friend std::ostream& operator<<(std::ostream& os, const Bit_range& range) {
        os << "{" << range.get_msb() << "," << range.get_lsb() << "}";
        return os;
    }

  private:
    const std::size_t m_msb;
    const std::size_t m_lsb;
    const std::size_t m_width;
};

constexpr bool is_overlapping(const Bit_range& lhs, const Bit_range& rhs) {
  return lhs.get_lsb() <= rhs.get_msb() && rhs.get_lsb() <= lhs.get_msb();
}

template <typename T>
constexpr T make_mask(std::size_t len, std::size_t pos = 0) {
  return ((static_cast<T>(1) << len)-1) << pos;
}

template<typename T>
concept Right_shiftable = requires(T a) {
  a >> 1;
};

template<typename T>
concept Andable = requires(T a) {
  a & static_cast<T>(1);
};

template <typename T>
constexpr T sign_extend(T data, std::size_t sign_pos) {
  assert(sign_pos < (sizeof(data) * 8));

  T m{T{1} << (sign_pos-1)};
  return (data ^ m) - m;
}

template <typename T> requires Right_shiftable<T> && Andable<T>
constexpr T extract_bits(T data, Bit_range range, bool sext = false) {
  assert(range.get_msb() < (sizeof(T) * CHAR_BIT));
  T unextended{(data >> range.get_lsb()) & static_cast<T>(make_mask<unsigned long long>(range.get_width()))};
  if (sext) return sign_extend(unextended, range.get_width() - 1);
  else      return unextended;
}

template <typename T> requires Right_shiftable<T> && Andable<T>
constexpr T extract_bits(T data, std::size_t pos, bool sext = false) {
  return extract_bits(data, {pos, pos}, sext);
}

template <typename T>
concept Left_shiftable = requires(T a) {
  a << 1;
};

template <typename T>
concept Shiftable = Left_shiftable<T> && Right_shiftable<T>;

template <typename T>
concept Orable = requires(T a) {
  a | static_cast<T>(1);
};

template <typename T> requires Shiftable<T> && Orable<T>
constexpr T extract_bits(T data, std::initializer_list<Bit_range> bit_ranges, bool sext = false) {
  T result{extract_bits(data, *(bit_ranges.begin()), sext)};
  for (const auto *it{bit_ranges.begin() + 1}; it != bit_ranges.end(); ++it) {
    result = (result << (*it).get_width()) | extract_bits(data, *it);
  }
  return result;
}
