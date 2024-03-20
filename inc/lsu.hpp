#pragma once

#include "riscv.hpp"
#include "riscv_algos.hpp"

#include <cstddef>
#include <cassert>

namespace Lsu {
  enum class Op {
    b,
    bu,
    h,
    hu,
    w
  };

  constexpr Uxlen transform_data(Op op, std::size_t addr, Uxlen data) {
    switch (op) {
      case Op::b:
        switch (addr & 0b11) {
          case 0: return extract_bits(data, { 7,  0}, true);
          case 1: return extract_bits(data, {15,  8}, true);
          case 2: return extract_bits(data, {23, 16}, true);
          case 3: return extract_bits(data, {31, 24}, true);
        }

      case Op::h:
        return (addr & 0b10) ? extract_bits(data, {31, 16}, true) :
            extract_bits(data, {15, 0}, true);

      case Op::w: return data;

      case Op::bu:
        switch (addr & 0b11) {
          case 0: return extract_bits(data, { 7,  0}, false);
          case 1: return extract_bits(data, {15,  8}, false);
          case 2: return extract_bits(data, {23, 16}, false);
          case 3: return extract_bits(data, {31, 24}, false);
        }

      case Op::hu:
        return (addr & 0b10) ? extract_bits(data, {31, 16}, false) :
            extract_bits(data, {15, 0}, false);
    }

    assert(0 && "Invalid op");
  }

  constexpr unsigned int get_be(Op op, std::size_t addr) {
    switch (op) {
      case Op::b: return 1 << (addr & 0b11);
      case Op::h: return (addr & 0b10) ? 0b1100 : 0b0011;

      default: assert(0 && "Other case items have no sense");
    }
  }
}
