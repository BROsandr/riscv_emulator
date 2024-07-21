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

  [[nodiscard("PURE FUN")]] inline bool is_misaligned(Op op, std::size_t addr) {
    switch (op) {
      case Op::bu: case Op::b: return false;
      case Op::hu: case Op::h: return addr & 0b01;
      case Op::w             : return addr & 0b11;
    }
    assert(0 && "Illegal lsu op");
  }

  [[nodiscard("PURE FUN")]] inline Uxlen transform_data(Op op, std::size_t addr, Uxlen data) {
    switch (op) {
      case Op::b:
        switch (addr & 0b11) {
          case 0: return extract_bits(data, { 7,  0}, true);
          case 1: return extract_bits(data, {15,  8}, true);
          case 2: return extract_bits(data, {23, 16}, true);
          case 3: return extract_bits(data, {31, 24}, true);
        }

      case Op::h:
        assert(!(addr & 0b01) && "Misalignment");
        return (addr & 0b10) ? extract_bits(data, {31, 16}, true) :
            extract_bits(data, {15, 0}, true);

      case Op::w:
        assert(!(addr & 0b11) && "Misalignment");
        return data;

      case Op::bu:
        switch (addr & 0b11) {
          case 0: return extract_bits(data, { 7,  0}, false);
          case 1: return extract_bits(data, {15,  8}, false);
          case 2: return extract_bits(data, {23, 16}, false);
          case 3: return extract_bits(data, {31, 24}, false);
        }

      case Op::hu:
        assert(!(addr & 0b01) && "Misalignment");
        return (addr & 0b10) ? extract_bits(data, {31, 16}, false) :
            extract_bits(data, {15, 0}, false);
    }

    assert(0 && "Invalid op");
  }

  [[nodiscard("PURE FUN")]] inline unsigned int get_be(Op op, std::size_t addr) {
    const auto byte_offset{addr & 0b11};
    switch (op) {
      case Op::b: case Op::bu: return 1u      << byte_offset;
      case Op::h: case Op::hu: return 0b0011u << byte_offset;
      case Op::w             : return 0xfu    << byte_offset;
    }
    assert(0 && "Illegal lsu op");
  }
}

[[nodiscard("PURE FUN")]] inline std::string to_string(Lsu::Op op) {
  using enum Lsu::Op;
  switch (op) {
    case b : return "b";
    case bu: return "bu";
    case h : return "h";
    case hu: return "hu";
    case w : return "w";
  }

  assert(0 && "Illegal lsu op");
}
